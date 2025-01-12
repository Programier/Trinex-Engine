/*
   AngelCode Scripting Library
   Copyright (c) 2003-2023 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


#include <new>
#include "as_config.h"
#include "as_scriptengine.h"
#include "as_scriptobject.h"
#include "as_texts.h"

BEGIN_AS_NAMESPACE

int asIScriptObject::GetTypeId() const
{
    auto ot = reinterpret_cast<asCObjectType*>(GetObjectType());
	asCDataType dt = asCDataType::CreateType(ot, false);
	return ot->engine->GetTypeIdFromDataType(dt);
}

unsigned int asIScriptObject::GetPropertyCount() const
{
	return GetObjectType()->GetPropertyCount();
}

int asIScriptObject::GetPropertyTypeId(unsigned int prop) const
{
	auto ot = reinterpret_cast<asCObjectType*>(GetObjectType());
    if( prop >= ot->properties.GetLength() )
		return asINVALID_ARG;

	return ot->engine->GetTypeIdFromDataType(ot->properties[prop]->type);
}

const char* asIScriptObject::GetPropertyName(unsigned int prop) const
{
	auto ot = reinterpret_cast<asCObjectType*>(GetObjectType());

	if (prop >= ot->properties.GetLength())
		return 0;

	return ot->properties[prop]->name.AddressOf();
}

void* asIScriptObject::GetAddressOfProperty(unsigned int prop)
{
	auto ot = reinterpret_cast<asCObjectType*>(GetObjectType());
    if( prop >= ot->properties.GetLength() )
		return 0;

	// Objects are stored by reference, so this must be dereferenced
    asCDataType *dt = &ot->properties[prop]->type;
    if( dt->IsObject() && !dt->IsObjectHandle() &&
        (dt->IsReference() || dt->GetTypeInfo()->flags & asOBJ_REF) )
        return *(void**)(((char*)this) + ot->properties[prop]->byteOffset);

    return (void*)(((char*)this) + ot->properties[prop]->byteOffset);
}

asIScriptEngine* asIScriptObject::GetEngine() const
{
    return GetObjectType()->GetEngine();
}

unsigned int asIScriptObject::GetScriptDataOffset(asITypeInfo* ot) const
{
	if (!ot)
		ot = GetObjectType()->GetNativeBaseType();
	return ot ? ot->GetSize() : sizeof(asCScriptObjectData);
}

unsigned int asIScriptObject::GetNativeObjectSize(asITypeInfo* ot) const
{
	if(!ot)
		ot = GetObjectType()->GetNativeBaseType();
	return ot ? ot->GetSize() : 0;
}

void asIScriptObject::Destroy()
{
    reinterpret_cast<asCScriptObject*>(this)->Destruct();
}

asCScriptObjectData::asCScriptObjectData(asCObjectType* ot)
{
    objectType = ot;
    refCount.set(1);
    isDestructCalled = false;
    extra = 0;
    hasRefCountReachedZero = false;
    ot->AddRef();
}

int asCScriptObjectData::AddRef() const
{
    auto ot = objectType;

    if(ot->flags & asOBJ_NOCOUNT)
        return 1;

    // Warn in case the application tries to increase the refCount after it has reached zero.
    // This may happen for example if the application calls a method on the class while it is
    // being destroyed. The application shouldn't do this because it may cause application
    // crashes if members that have already been destroyed are accessed accidentally.
    if( hasRefCountReachedZero )
    {
        if( ot && ot->engine )
        {
            asCString msg;
            msg.Format(TXT_RESURRECTING_SCRIPTOBJECT_s, ot->name.AddressOf());
            ot->engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
        }
    }

    // Increase counter and clear flag set by GC
    gcFlag = false;
    return refCount.atomicInc();
}

int asCScriptObjectData::Release() const
{
    auto ot = objectType;

    if(ot->flags & asOBJ_NOCOUNT)
        return 1;

    // Clear the flag set by the GC
    gcFlag = false;

    // If the weak ref flag exists it is because someone held a weak ref
    // and that someone may add a reference to the object at any time. It
    // is ok to check the existance of the weakRefFlag without locking here
    // because if the refCount is 1 then no other thread is currently
    // creating the weakRefFlag.
    if( refCount.get() == 1 && extra && extra->weakRefFlag )
    {
        // Set the flag to tell others that the object is no longer alive
        // We must do this before decreasing the refCount to 0 so we don't
        // end up with a race condition between this thread attempting to
        // destroy the object and the other that temporary added a strong
        // ref from the weak ref.
        extra->weakRefFlag->Set(true);
    }

    // Call the script destructor behaviour if the reference counter is 1.
    if( refCount.get() == 1 && !isDestructCalled )
    {
        // This cast is OK since we are the last reference
        reinterpret_cast<asCScriptObject*>(const_cast<asCScriptObjectData*>(this))->CallDestructor(objectType);
    }

    // Now do the actual releasing
    int r = refCount.atomicDec();
    if( r == 0 )
    {
        // Flag this object as being destroyed so the application
        // can be warned if the code attempts to resurrect the object
        // during the destructor. This also avoids a recursive call
        // to the destructor which would crash the application if it
        // really does resurrect the object.
        if( !hasRefCountReachedZero )
        {
            hasRefCountReachedZero = true;

            // This cast is OK since we are the last reference
            reinterpret_cast<asCScriptObject*>(const_cast<asCScriptObjectData*>(this))->Destruct();
        }
        return 0;
    }

    return r;
}

asILockableSharedBool* asCScriptObjectData::GetWeakRefFlag() const
{
    // If the object's refCount has already reached zero then the object is already
    // about to be destroyed so it's ok to return null if the weakRefFlag doesn't already
    // exist
    if( (extra && extra->weakRefFlag) || hasRefCountReachedZero )
        return extra->weakRefFlag;

    // Lock globally so no other thread can attempt
    // to create a shared bool at the same time.
    // TODO: runtime optimize: Instead of locking globally, it would be possible to have
    //                         a critical section per object type. This would reduce the
    //                         chances of two threads lock on the same critical section.
    asAcquireExclusiveLock();

    // Make sure another thread didn't create the
    // flag while we waited for the lock
    if( !extra )
        extra = asNEW(SExtra);
    if( !extra->weakRefFlag )
        extra->weakRefFlag = asNEW(asCLockableSharedBool);

    asReleaseExclusiveLock();

    return extra->weakRefFlag;
}

int asCScriptObjectData::GetRefCount()
{
    return refCount.get();
}

void asCScriptObjectData::SetGCFlag()
{
    gcFlag = true;
}

bool asCScriptObjectData::GetGCFlag()
{
    return gcFlag;
}

asITypeInfo* asCScriptObjectData::GetObjectType() const
{
    return objectType;
}

int asCScriptObjectData::CopyFrom(const asIScriptObject* other)
{
    return reinterpret_cast<asCScriptObject*>(this)->CopyFromAs(static_cast<const asCScriptObject*>(other), objectType);
}

asCScriptObjectData::~asCScriptObjectData()
{
	auto ot = objectType;

	if (extra)
	{
		if (extra->weakRefFlag)
		{
			extra->weakRefFlag->Release();
			extra->weakRefFlag = 0;
		}

		asDELETE(extra, SExtra);
	}

	ot->Release();

	if (!(ot->flags & asOBJ_NOCOUNT))
	{
		// Something is really wrong if the refCount is not 0 by now
		asASSERT(refCount.get() == 0);
	}
}

// This helper function will call the default factory, that is a script function
asIScriptObject *ScriptObjectFactory(const asCObjectType *objType, asCScriptEngine *engine)
{
    asIScriptContext *ctx = 0;
    int r = 0;
    bool isNested = false;

	// Use nested call in the context if there is an active context
	ctx = asGetActiveContext();
    if( ctx )
	{
        // It may not always be possible to reuse the current context,
		// in which case we'll have to create a new one any way.
        if( ctx->GetEngine() == objType->GetEngine() && ctx->PushState() == asSUCCESS )
			isNested = true;
		else
			ctx = 0;
	}

    if( ctx == 0 )
	{
		// Request a context from the engine
		ctx = engine->RequestContext();
        if( ctx == 0 )
		{
			// TODO: How to best report this failure?
			return 0;
		}
	}

	r = ctx->Prepare(engine->scriptFunctions[objType->beh.factory]);
    if( r < 0 )
	{
        if( isNested )
			ctx->PopState();
		else
			engine->ReturnContext(ctx);
		return 0;
	}

    for(;;)
	{
		r = ctx->Execute();

        // We can't allow this execution to be suspended
		// so resume the execution immediately
        if( r != asEXECUTION_SUSPENDED )
			break;
	}

    if( r != asEXECUTION_FINISHED )
	{
        if( isNested )
		{
			ctx->PopState();

			// If the execution was aborted or an exception occurred,
			// then we should forward that to the outer execution.
            if( r == asEXECUTION_EXCEPTION )
			{
				// TODO: How to improve this exception
				ctx->SetException(TXT_EXCEPTION_IN_NESTED_CALL);
			}
            else if( r == asEXECUTION_ABORTED )
				ctx->Abort();
		}
		else
			engine->ReturnContext(ctx);
		return 0;
	}

    asIScriptObject *ptr = (asIScriptObject*)ctx->GetReturnAddress();

	// Increase the reference, because the context will release its pointer
	ptr->AddRef();

    if( isNested )
		ctx->PopState();
	else
		engine->ReturnContext(ctx);

	return ptr;
}

// This helper function will call the copy factory, that is a script function
// TODO: Clean up: This function is almost identical to ScriptObjectFactory. Should make better use of the identical code.
asIScriptObject *ScriptObjectCopyFactory(const asCObjectType *objType, void *origObj, asCScriptEngine *engine)
{
    asIScriptContext *ctx = 0;
    int r = 0;
    bool isNested = false;

	// Use nested call in the context if there is an active context
	ctx = asGetActiveContext();
	if (ctx)
	{
        // It may not always be possible to reuse the current context,
		// in which case we'll have to create a new one any way.
		if (ctx->GetEngine() == objType->GetEngine() && ctx->PushState() == asSUCCESS)
			isNested = true;
		else
			ctx = 0;
	}

	if (ctx == 0)
	{
		// Request a context from the engine
		ctx = engine->RequestContext();
		if (ctx == 0)
		{
			// TODO: How to best report this failure?
			return 0;
		}
	}

	r = ctx->Prepare(engine->scriptFunctions[objType->beh.copyfactory]);
	if (r < 0)
	{
		if (isNested)
			ctx->PopState();
		else
			engine->ReturnContext(ctx);
		return 0;
	}

	// Let the context handle the case for argument by ref (&) or by handle (@)
	ctx->SetArgObject(0, origObj);

	for (;;)
	{
		r = ctx->Execute();

        // We can't allow this execution to be suspended
		// so resume the execution immediately
		if (r != asEXECUTION_SUSPENDED)
			break;
	}

	if (r != asEXECUTION_FINISHED)
	{
		if (isNested)
		{
			ctx->PopState();

			// If the execution was aborted or an exception occurred,
			// then we should forward that to the outer execution.
			if (r == asEXECUTION_EXCEPTION)
			{
				// TODO: How to improve this exception
				ctx->SetException(TXT_EXCEPTION_IN_NESTED_CALL);
			}
			else if (r == asEXECUTION_ABORTED)
				ctx->Abort();
		}
		else
			engine->ReturnContext(ctx);
		return 0;
	}

    asIScriptObject *ptr = (asIScriptObject*)ctx->GetReturnAddress();

	// Increase the reference, because the context will release its pointer
	ptr->AddRef();

	if (isNested)
		ctx->PopState();
	else
		engine->ReturnContext(ctx);

	return ptr;
}

#if defined(AS_MAX_PORTABILITY) || defined(AS_NO_CLASS_METHODS)

static void ScriptObject_AddRef_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
	self->AddRef();
}

static void ScriptObject_Release_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
	self->Release();
}

static void ScriptObject_GetRefCount_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
    *(int*)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ScriptObject_SetFlag_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
	self->SetFlag();
}

static void ScriptObject_GetFlag_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
    *(bool*)gen->GetAddressOfReturnLocation() = self->GetFlag();
}

static void ScriptObject_GetWeakRefFlag_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
    *(asILockableSharedBool**)gen->GetAddressOfReturnLocation() = self->GetWeakRefFlag();
}

static void ScriptObject_EnumReferences_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
    asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ScriptObject_ReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();
    asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

static void ScriptObject_Assignment_Generic(asIScriptGeneric *gen)
{
    asCScriptObject *other = *(asCScriptObject**)gen->GetAddressOfArg(0);
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();

	*self = *other;

    *(asCScriptObject**)gen->GetAddressOfReturnLocation() = self;
}

static void ScriptObject_Construct_Generic(asIScriptGeneric *gen)
{
    asCObjectType *objType = *(asCObjectType**)gen->GetAddressOfArg(0);
    asCScriptObject *self = (asCScriptObject*)gen->GetObject();

	ScriptObject_Construct(objType, self);
}

#endif

void RegisterScriptObject(asCScriptEngine *engine)
{
	// Register the default script class behaviours
	int r = 0;
    UNUSED_VAR(r); // It is only used in debug mode
	engine->scriptTypeBehaviours.engine = engine;
    engine->scriptTypeBehaviours.flags = asOBJ_SCRIPT_OBJECT | asOBJ_REF | asOBJ_GC;
    engine->scriptTypeBehaviours.name = "$obj";
#if !defined(AS_MAX_PORTABILITY) && !defined(AS_NO_CLASS_METHODS)
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptObject_Construct), asCALL_CDECL_OBJLAST, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_ADDREF, "void f()", asMETHOD(asCScriptObject,AddRef), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_RELEASE, "void f()", asMETHOD(asCScriptObject,Release), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterMethodToObjectType(&engine->scriptTypeBehaviours, "int &opAssign(int &in)", asFUNCTION(ScriptObject_Assignment), asCALL_CDECL_OBJLAST); asASSERT( r >= 0 );

	// Weakref behaviours
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asMETHOD(asCScriptObject,GetWeakRefFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );

	// Register GC behaviours
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(asCScriptObject,GetRefCount), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_SETGCFLAG, "void f()", asMETHOD(asCScriptObject,SetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(asCScriptObject,GetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(asCScriptObject,EnumReferences), asCALL_THISCALL, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(asCScriptObject,ReleaseAllHandles), asCALL_THISCALL, 0); asASSERT( r >= 0 );
#else
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptObject_Construct_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptObject_AddRef_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptObject_Release_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterMethodToObjectType(&engine->scriptTypeBehaviours, "int &opAssign(int &in)", asFUNCTION(ScriptObject_Assignment_Generic), asCALL_GENERIC); asASSERT( r >= 0 );

	// Weakref behaviours
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asFUNCTION(ScriptObject_GetWeakRefFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );

	// Register GC behaviours
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptObject_GetRefCount_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptObject_SetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptObject_GetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptObject_EnumReferences_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
    r = engine->RegisterBehaviourToObjectType(&engine->scriptTypeBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptObject_ReleaseAllHandles_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
#endif
}

void ScriptObject_Construct(asCObjectType *objType, asCScriptObject *self)
{
    if(!objType->GetNativeBaseType())
        new (self) asCScriptObjectData(objType);
    self->init(objType, true);
}

void ScriptObject_ConstructUnitialized(asCObjectType *objType, asCScriptObject *self)
{
    if(!objType->GetNativeBaseType())
        new (self) asCScriptObjectData(objType);
    self->init(objType, false);
}

void asCScriptObject::init(asCObjectType *ot, bool doInitialize)
{
	// Notify the garbage collector of this object
    if( ot->flags & asOBJ_GC )
		ot->engine->gc.AddScriptObjectToGC(this, ot);

	// Initialize members to zero. Technically we only need to zero the pointer
	// members, but just the memset is faster than having to loop and check the datatypes

	{
		auto offset = GetScriptDataOffset(ot);
		asBYTE* mem = reinterpret_cast<asBYTE*>(this) + offset;
        memset(mem, 0, ot->size - offset);
	}

    if( doInitialize )
	{
#ifdef AS_NO_MEMBER_INIT
		// When member initialization is disabled the constructor must make sure
		// to allocate and initialize all members with the default constructor
        for( asUINT n = 0; n < objType->properties.GetLength(); n++ )
		{
            asCObjectProperty *prop = objType->properties[n];

            if(prop->isNative)
				continue;

            if( prop->type.IsObject() && !prop->type.IsObjectHandle() )
			{
                if( prop->type.IsReference() || prop->type.GetTypeInfo()->flags & asOBJ_REF )
				{
                    asPWORD *ptr = reinterpret_cast<asPWORD*>(reinterpret_cast<asBYTE*>(this) + prop->byteOffset);
                    if( prop->type.GetTypeInfo()->flags & asOBJ_SCRIPT_OBJECT )
                        *ptr = (asPWORD)ScriptObjectFactory(prop->type.GetTypeInfo(), ot->engine);
					else
                        *ptr = (asPWORD)AllocateUninitializedObject(prop->type.GetTypeInfo(), ot->engine);
				}
			}
		}
#endif
	}
	else
	{
		// When the object is created without initialization, all non-handle members must be allocated, but not initialized
        asCScriptEngine *engine = ot->engine;
        for( asUINT n = 0; n < ot->properties.GetLength(); n++ )
		{
            asCObjectProperty *prop = ot->properties[n];

            if(prop->isNative)
				continue;

            if( prop->type.IsObject() && !prop->type.IsObjectHandle() )
			{
                if( prop->type.IsReference() || (prop->type.GetTypeInfo()->flags & asOBJ_REF) )
				{
                    asPWORD *ptr = reinterpret_cast<asPWORD*>(reinterpret_cast<asBYTE*>(this) + prop->byteOffset);
                    *ptr = (asPWORD)AllocateUninitializedObject(CastToObjectType(prop->type.GetTypeInfo()), engine);
				}
			}
		}
	}
}

void asCScriptObject::ExecDestructor()
{
    auto ot = objType();

    // The engine pointer should be available from the objectType
    asCScriptEngine *engine = ot->engine;

    // Destroy all properties
    // In most cases the members are initialized in the order they have been declared,
    // so it's safer to uninitialize them from last to first. The order may be different
    // depending on the use of inheritance and or initialization in the declaration.
    // TODO: Should the order of initialization be stored by the compiler so that the
    //       reverse order can be guaranteed during the destruction?
    for( int n = (int)ot->properties.GetLength()-1; n >= 0; n-- )
    {
        asCObjectProperty *prop = ot->properties[n];

        if(prop->isNative)
            continue;

        if( prop->type.IsObject() )
        {
            // Destroy the object
            asCObjectType *propType = CastToObjectType(prop->type.GetTypeInfo());
            if( prop->type.IsReference() || propType->flags & asOBJ_REF )
            {
                void **ptr = (void**)(((char*)this) + prop->byteOffset);
                if( *ptr )
                {
                    reinterpret_cast<asCScriptObject*>(this)->FreeObject(*ptr, propType, engine);
                    *(asDWORD*)ptr = 0;
                }
            }
            else
            {
                // The object is allocated inline. As only POD objects may be allocated inline
                // it is not a problem to call the destructor even if the object may never have
                // been initialized, e.g. if an exception interrupted the constructor.
                asASSERT( propType->flags & asOBJ_POD );

                void *ptr = (void**)(((char*)this) + prop->byteOffset);
                if( propType->beh.destruct )
                    engine->CallObjectMethod(ptr, propType->beh.destruct);
            }
        }
        else if( prop->type.IsFuncdef() )
        {
            // Release the function descriptor
            asCScriptFunction **ptr = (asCScriptFunction**)(((char*)this) + prop->byteOffset);
            if (*ptr)
            {
                (*ptr)->Release();
                *ptr = 0;
            }
        }
    }

    // Call the destructor, which will also call the GCObject's destructor
    static_cast<asIScriptObject*>(this)->~asIScriptObject();
}

void asCScriptObject::Destruct()
{
    ExecDestructor();
	asCScriptEngine::CallFree(this);
}

void asCScriptObject::CallDestructor(asCObjectType* type)
{
	// Only allow the destructor to be called once
    //if( GetDataBlock()->isDestructCalled ) return;

    asIScriptContext *ctx = 0;
    bool isNested = false;
    bool doAbort = false;

    // Make sure the destructor is called once only, even if the
	// reference count is increased and then decreased again
    //GetDataBlock()->isDestructCalled = true;

	// Call the destructor for this class and all the super classes
    asCObjectType *ot = type;
    while( ot )
	{
		int funcIndex = ot->beh.destruct;
        if( funcIndex )
		{
            if( ctx == 0 )
			{
				// Check for active context first as it is quicker
				// to reuse than to set up a new one.
				ctx = asGetActiveContext();
                if( ctx )
				{
                    if( ctx->GetEngine() == type->GetEngine() && ctx->PushState() == asSUCCESS )
						isNested = true;
					else
						ctx = 0;
				}

                if( ctx == 0 )
				{
					// Request a context from the engine
					ctx = type->engine->RequestContext();
                    if( ctx == 0 )
					{
						// TODO: How to best report this failure?
						return;
					}
				}
			}

			int r = ctx->Prepare(type->engine->scriptFunctions[funcIndex]);
            if( r >= 0 )
			{
				ctx->SetObject(this);

                for(;;)
				{
					r = ctx->Execute();

					// If the script tries to suspend itself just restart it
                    if( r != asEXECUTION_SUSPENDED )
						break;
				}

				// Exceptions in the destructor will be ignored, as there is not much
				// that can be done about them. However a request to abort the execution
				// will be forwarded to the outer execution, in case of a nested call.
                if( r == asEXECUTION_ABORTED )
					doAbort = true;

				// Observe, even though the current destructor was aborted or an exception
				// occurred, we still try to execute the base class' destructor if available
				// in order to free as many resources as possible.
			}
		}

		ot = ot->derivedFrom;
	}

    if( ctx )
	{
        if( isNested )
		{
			ctx->PopState();

			// Forward any request to abort the execution to the outer call
            if( doAbort )
				ctx->Abort();
		}
		else
		{
			// Return the context to engine
			type->engine->ReturnContext(ctx);
		}
	}
}

asCObjectType* asCScriptObject::objType() const
{
    return reinterpret_cast<asCObjectType*>(GetObjectType());
}

// interface

void asCScriptObject::EnumReferences(asIScriptEngine *engine)
{
	auto ot = objType();
	// We'll notify the GC of all object handles that we're holding
    for( asUINT n = 0; n < ot->properties.GetLength(); n++ )
	{
        asCObjectProperty *prop = ot->properties[n];
        void *ptr = 0;
		if (prop->type.IsObject())
		{
			if (prop->type.IsReference() || (prop->type.GetTypeInfo()->flags & asOBJ_REF))
                ptr = *(void**)(((char*)this) + prop->byteOffset);
			else
                ptr = (void*)(((char*)this) + prop->byteOffset);

			// The members of the value type needs to be enumerated
			// too, since the value type may be holding a reference.
			if ((prop->type.GetTypeInfo()->flags & asOBJ_VALUE) && (prop->type.GetTypeInfo()->flags & asOBJ_GC))
			{
                reinterpret_cast<asCScriptEngine*>(engine)->CallObjectMethod(ptr, engine, CastToObjectType(prop->type.GetTypeInfo())->beh.gcEnumReferences);
			}
		}
		else if (prop->type.IsFuncdef())
            ptr = *(void**)(((char*)this) + prop->byteOffset);

		if (ptr)
            ((asCScriptEngine*)engine)->GCEnumCallback(ptr);
	}
}

void asCScriptObject::ReleaseAllHandles(asIScriptEngine *engine)
{
	auto ot = objType();
    for( asUINT n = 0; n < ot->properties.GetLength(); n++ )
	{
        asCObjectProperty *prop = ot->properties[n];

		if (prop->type.IsObject())
		{
			if (prop->type.IsObjectHandle())
			{
                void **ptr = (void**)(((char*)this) + prop->byteOffset);
				if (*ptr)
				{
					asASSERT((prop->type.GetTypeInfo()->flags & asOBJ_NOCOUNT) || prop->type.GetBehaviour()->release);
					if (prop->type.GetBehaviour()->release)
                        ((asCScriptEngine*)engine)->CallObjectMethod(*ptr, prop->type.GetBehaviour()->release);
					*ptr = 0;
				}
			}
			else if ((prop->type.GetTypeInfo()->flags & asOBJ_VALUE) && (prop->type.GetTypeInfo()->flags & asOBJ_GC))
			{
				// The members of the members needs to be released
				// too, since they may be holding a reference. Even
				// if the member is a value type.
                void *ptr = 0;
				if (prop->type.IsReference())
                    ptr = *(void**)(((char*)this) + prop->byteOffset);
				else
                    ptr = (void*)(((char*)this) + prop->byteOffset);

                reinterpret_cast<asCScriptEngine*>(engine)->CallObjectMethod(ptr, engine, CastToObjectType(prop->type.GetTypeInfo())->beh.gcReleaseAllReferences);
			}
		}
		else if (prop->type.IsFuncdef())
		{
            asCScriptFunction **ptr = (asCScriptFunction**)(((char*)this) + prop->byteOffset);
			if (*ptr)
			{
				(*ptr)->Release();
				*ptr = 0;
			}
		}
	}
}

asCScriptObject &ScriptObject_Assignment(asCScriptObject *other, asCScriptObject *self)
{
	return (*self = *other);
}

asCScriptObject &asCScriptObject::operator=(const asCScriptObject &other)
{
	CopyFromAs(&other, objType());
	return *this;
}

// internal
int asCScriptObject::CopyFromAs(const asCScriptObject *other, asCObjectType *in_objType)
{
    if( other != this )
	{
        if( !other->objType()->DerivesFrom(in_objType) )
		{
            // We cannot allow a value assignment from a type that isn't the same or
			// derives from this type as the member properties may not have the same layout
            asIScriptContext *ctx = asGetActiveContext();
			ctx->SetException(TXT_MISMATCH_IN_VALUE_ASSIGN);
			return asERROR;
		}

		// If the script class implements the opAssign method, it should be called
        asCScriptEngine *engine = in_objType->engine;
        asCScriptFunction *func = engine->scriptFunctions[in_objType->beh.copy];
        if( func->funcType == asFUNC_SYSTEM )
		{
			// If derived, use the base class' assignment operator to copy the inherited
			// properties. Then only copy new properties for the derived class
            if( in_objType->derivedFrom )
				CopyFromAs(other, in_objType->derivedFrom);

            for( asUINT n = in_objType->derivedFrom ? in_objType->derivedFrom->properties.GetLength() : 0;
                 n < in_objType->properties.GetLength();
                 n++ )
			{
                asCObjectProperty *prop = in_objType->properties[n];
                if( prop->type.IsObject() )
				{
                    void **dst = (void**)(((char*)this) + prop->byteOffset);
                    void **src = (void**)(((char*)other) + prop->byteOffset);
                    if( !prop->type.IsObjectHandle() )
					{
                        if( prop->type.IsReference() || (prop->type.GetTypeInfo()->flags & asOBJ_REF) )
							CopyObject(*src, *dst, CastToObjectType(prop->type.GetTypeInfo()), engine);
						else
							CopyObject(src, dst, CastToObjectType(prop->type.GetTypeInfo()), engine);
					}
					else
                        CopyHandle((asPWORD*)src, (asPWORD*)dst, CastToObjectType(prop->type.GetTypeInfo()), engine);
				}
				else if (prop->type.IsFuncdef())
				{
                    asCScriptFunction **dst = (asCScriptFunction**)(((char*)this) + prop->byteOffset);
                    asCScriptFunction **src = (asCScriptFunction**)(((char*)other) + prop->byteOffset);
					if (*dst)
						(*dst)->Release();
					*dst = *src;
					if (*dst)
						(*dst)->AddRef();
				}
				else
				{
                    void *dst = ((char*)this) + prop->byteOffset;
                    void *src = ((char*)other) + prop->byteOffset;
					memcpy(dst, src, prop->type.GetSizeInMemoryBytes());
				}
			}
		}
		else
		{
			// Reuse the active context or create a new one to call the script class' opAssign method
            asIScriptContext *ctx = 0;
            int r = 0;
            bool isNested = false;

			ctx = asGetActiveContext();
            if( ctx )
			{
                if( ctx->GetEngine() == engine && ctx->PushState() == asSUCCESS )
					isNested = true;
				else
					ctx = 0;
			}

            if( ctx == 0 )
			{
				// Request a context from the engine
				ctx = engine->RequestContext();
                if( ctx == 0 )
					return asERROR;
			}

			r = ctx->Prepare(engine->scriptFunctions[in_objType->beh.copy]);
            if( r < 0 )
			{
                if( isNested )
					ctx->PopState();
				else
					engine->ReturnContext(ctx);
				return r;
			}

			r = ctx->SetArgAddress(0, const_cast<asCScriptObject*>(other));
            asASSERT( r >= 0 );
			r = ctx->SetObject(this);
            asASSERT( r >= 0 );

            for(;;)
			{
				r = ctx->Execute();

                // We can't allow this execution to be suspended
				// so resume the execution immediately
                if( r != asEXECUTION_SUSPENDED )
					break;
			}

            if( r != asEXECUTION_FINISHED )
			{
                if( isNested )
				{
					ctx->PopState();

					// If the execution was aborted or an exception occurred,
					// then we should forward that to the outer execution.
                    if( r == asEXECUTION_EXCEPTION )
					{
						// TODO: How to improve this exception
						ctx->SetException(TXT_EXCEPTION_IN_NESTED_CALL);
					}
                    else if( r == asEXECUTION_ABORTED )
						ctx->Abort();
				}
				else
				{
					// Return the context to the engine
					engine->ReturnContext(ctx);
				}
				return asERROR;
			}

            if( isNested )
				ctx->PopState();
			else
			{
				// Return the context to the engine
				engine->ReturnContext(ctx);
			}
		}
	}

	return asSUCCESS;
}

int asIScriptObject::CopyFrom(const asIScriptObject *other)
{
    if( other == 0 ) return asINVALID_ARG;

    if( GetTypeId() != other->GetTypeId() )
		return asINVALID_TYPE;

    *this = *(asCScriptObject*)other;

	return asSUCCESS;
}

void *asCScriptObject::AllocateUninitializedObject(asCObjectType *in_objType, asCScriptEngine *engine)
{
    void *ptr = 0;

    if( in_objType->flags & asOBJ_SCRIPT_OBJECT )
	{
		ptr = engine->CallAlloc(in_objType);
		ScriptObject_ConstructUnitialized(in_objType, reinterpret_cast<asCScriptObject*>(ptr));
	}
    else if( in_objType->flags & asOBJ_TEMPLATE )
	{
		// Templates store the original factory that takes the object
		// type as a hidden parameter in the construct behaviour
		ptr = engine->CallGlobalFunctionRetPtr(in_objType->beh.construct, in_objType);
	}
    else if( in_objType->flags & asOBJ_REF )
	{
		ptr = engine->CallGlobalFunctionRetPtr(in_objType->beh.factory);
	}
	else
	{
        ptr = engine->CallAlloc(in_objType);
		int funcIndex = in_objType->beh.construct;
        if( funcIndex )
			engine->CallObjectMethod(ptr, funcIndex);
	}

	return ptr;
}

void asCScriptObject::FreeObject(void *ptr, asCObjectType *in_objType, asCScriptEngine *engine)
{
    if( in_objType->flags & asOBJ_REF )
	{
        asASSERT( (in_objType->flags & asOBJ_NOCOUNT) || in_objType->beh.release );
        if(in_objType->beh.release )
			engine->CallObjectMethod(ptr, in_objType->beh.release);
	}
	else
	{
        if( in_objType->beh.destruct )
			engine->CallObjectMethod(ptr, in_objType->beh.destruct);

		engine->CallFree(ptr);
	}
}

void asCScriptObject::CopyObject(const void *src, void *dst, asCObjectType *in_objType, asCScriptEngine *engine)
{
	int funcIndex = in_objType->beh.copy;
    if( funcIndex )
	{
        asCScriptFunction *func = engine->scriptFunctions[in_objType->beh.copy];
        if( func->funcType == asFUNC_SYSTEM )
			engine->CallObjectMethod(dst, const_cast<void*>(src), funcIndex);
		else
		{
			// Call the script class' opAssign method
            asASSERT(in_objType->flags & asOBJ_SCRIPT_OBJECT );
			reinterpret_cast<asCScriptObject*>(dst)->CopyFrom(reinterpret_cast<const asCScriptObject*>(src));
		}
	}
    else if( in_objType->size && (in_objType->flags & asOBJ_POD) )
		memcpy(dst, src, in_objType->size);
}

void asCScriptObject::CopyHandle(asPWORD *src, asPWORD *dst, asCObjectType *in_objType, asCScriptEngine *engine)
{
	// asOBJ_NOCOUNT doesn't have addref or release behaviours
	asASSERT((in_objType->flags & asOBJ_NOCOUNT) || (in_objType->beh.release && in_objType->beh.addref));

	if (*dst && in_objType->beh.release)
		engine->CallObjectMethod(*(void**) dst, in_objType->beh.release);
	*dst = *src;
	if (*dst && in_objType->beh.addref)
		engine->CallObjectMethod(*(void**) dst, in_objType->beh.addref);
}

// TODO: weak: Should move to its own file
asCLockableSharedBool::asCLockableSharedBool() : value(false)
{
	refCount.set(1);
}

int asCLockableSharedBool::AddRef() const
{
	return refCount.atomicInc();
}

int asCLockableSharedBool::Release() const
{
	int r = refCount.atomicDec();
    if( r == 0 )
		asDELETE(const_cast<asCLockableSharedBool*>(this), asCLockableSharedBool);
	return r;
}

bool asCLockableSharedBool::Get() const
{
	return value;
}

void asCLockableSharedBool::Set(bool v)
{
    // Make sure the value is not changed while another thread
	// is inspecting it and taking a decision on what to do.
	Lock();
	value = v;
	Unlock();
}

void asCLockableSharedBool::Lock() const
{
	ENTERCRITICALSECTION(lock);
}

void asCLockableSharedBool::Unlock() const
{
	LEAVECRITICALSECTION(lock);
}

// Interface
// Auxiliary function to allow applications to create shared
// booleans without having to implement the logic for them
AS_API asILockableSharedBool *asCreateLockableSharedBool()
{
	return asNEW(asCLockableSharedBool);
}

END_AS_NAMESPACE

