#pragma once

#ifndef ANGELSCRIPT_H

#ifdef AS_USE_NAMESPACE
#define BEGIN_AS_NAMESPACE                                                                                                       \
    namespace AngelScript                                                                                                        \
    {
#define END_AS_NAMESPACE }
#define AS_NAMESPACE_QUALIFIER AngelScript::
#else
#define BEGIN_AS_NAMESPACE
#define END_AS_NAMESPACE
#define AS_NAMESPACE_QUALIFIER ::
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#if defined(ANGELSCRIPT_EXPORT)
#define AS_API __declspec(dllexport)
#elif defined(ANGELSCRIPT_DLL_LIBRARY_IMPORT)
#define AS_API __declspec(dllimport)
#else// statically linked library
#define AS_API
#endif
#elif defined(__GNUC__)
#if defined(ANGELSCRIPT_EXPORT)
#define AS_API __attribute__((visibility("default")))
#else
#define AS_API
#endif
#else
#define AS_API
#endif

#endif

BEGIN_AS_NAMESPACE

class asILockableSharedBool;
class asITypeInfo;
class asIScriptEngine;

class AS_API asIScriptObject
{
public:
	// Memory management
	virtual int AddRef() const                            = 0;
	virtual int Release() const                           = 0;
	virtual asILockableSharedBool* GetWeakRefFlag() const = 0;
    virtual int GetRefCount()                             = 0;
    virtual void SetGCFlag()                              = 0;
    virtual bool GetGCFlag()                              = 0;

	// Type info
	int GetTypeId() const;
	virtual asITypeInfo* GetObjectType() const = 0;

	// Class properties
	unsigned int GetPropertyCount() const;
	int GetPropertyTypeId(unsigned int prop) const;
	const char* GetPropertyName(unsigned int prop) const;
	void* GetAddressOfProperty(unsigned int prop);

	// Miscellaneous
    asIScriptEngine* GetEngine() const;
	virtual int CopyFrom(const asIScriptObject* other) = 0;
	void Destroy();

protected:
    unsigned int GetScriptDataOffset(class asITypeInfo* ot = nullptr) const;
    unsigned int GetNativeObjectSize(class asITypeInfo* ot = nullptr) const;
	virtual ~asIScriptObject()
	{}

    friend class asCScriptObject;
};

END_AS_NAMESPACE

#ifndef ANGELSCRIPT_H
#undef AS_API
#undef BEGIN_AS_NAMESPACE
#undef END_AS_NAMESPACE
#undef AS_NAMESPACE_QUALIFIER
#endif
