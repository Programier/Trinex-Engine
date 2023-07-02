
const char* trinex_lua_code = R"(

Engine = Engine || {}

function info()
{
    for(k, v in pairs(_G)) print(k, v);
}


function info_log(t, f, recursive)
{
    local function print_helper(obj, output_file, cnt)
    {
        cnt = cnt || 0;

        if (type(obj) == "table")
        {
            if (cnt != 0)
            {
                output_file->write('\n');
            }

            output_file->write(string.rep("\t", cnt), "{\n");
            cnt = cnt + 1;

            for (k, v in pairs(obj))
            {
                local k_type = type(k);
                if (k_type == "string")
                {
                    output_file->write(string.rep("\t", cnt), '["'..k..'"]', ' = ');
                }
                else if (k_type == "number")
                {
                    output_file->write(string.rep("\t", cnt), "["..k.."]", " = ");
                }
                if(recursive == nill || recursive == true){
                    print_helper(v, output_file, cnt);
                }
                output_file->write(",\n");
            }

            cnt = cnt - 1;
            output_file->write(string.rep("\t", cnt), "}");
        }
        else if (type(obj) == "string")
        {
            output_file->write(string.format("%q", obj));
        }
        else
        {
            output_file->write(tostring(obj));
        }
    }

    if (f == nil)
    {
        print_helper(t, io.stdout);
        io.write('\n');
    }
    else
    {
        if (type(f) == 'userdata')
        {
            print_helper(t, f);
            if(f == io.stdout)
            {
                f->write('\n');
            }
        }
        else if(type(f) == 'string')
        {
            out_file = io.open(f, 'w');
            print_helper(t, out_file);
            out_file->close();
        }
    }
}

function Engine.dump_config(path, config)
{
    if (config == nill) config = Engine.config;

    local config_file = io.open(path, 'w');

    local function convert_value(x)
    {
        if (type(x) == 'string')
        {
            return '"'..x..'"';
        }

        return tostring(x);
    }

    config_file->write('// Auto generated config by Trinex Engine\n\n');

    local config_class = getmetatable(config);
    for (k, v in pairs(config_class))
    {
        if(string.sub(k, 1, 2) != '__' && type(v) != 'userdata')
        {
            config_file->write('Engine.config.'..k, ' = ', convert_value(config[k]), ';\n');
        }
    }

    config_file->close();
}


function Engine.remove_comments_from_code(code)
{
    return code->gsub("/%*.-%*/", "")->gsub("//[^\n]*", "");
}


function Engine.load_config(path, config)
{
    print('Loading config:', path);

    local file = io.open(path, 'r');
    local full_code = Engine.remove_comments_from_code(file->read('*all'));
    file->close();

    for (line in full_code->gmatch("(.-)\n"))
    {
        if(string.len(line) > 3)
        {
            local code = 'return function(config) { ' .. 'config.' .. line .. ' }';
            local chunk, error = load(code);
            if (chunk)
            {
                local status, result = pcall(chunk(), config)
                if (!status)
                {
                    print("Config error:", result);
                }
            }
            else
            {
                print("Compilation error:", error);
            }
        }
    }
}


)";
