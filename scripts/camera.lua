local math = require('math');

local speed        = 5.0;
local min_time     = 100;
local max_time     = 0;
local current_diff = 0;
local K            = 0.5;
local frame        = 0;

function camera_update(camera)
{
    current_diff        = (current_diff * K) + (Engine.Event.diff_time() * (1.0 - K));
    local current_speed = speed * current_diff;
    local execute_time  = os.clock();
    frame               = frame + 1;

    if (Engine.Event.time() > 5.0)
    {
        min_time = math.min(min_time, current_diff);
        max_time = math.max(max_time, current_diff);
    }

    if (Engine.KeyboardEvent.pressed(Engine.Key.W))
    {
        camera.transform->move(camera.transform->front_vector() * current_speed, true);
    }

    if (Engine.KeyboardEvent.pressed(Engine.Key.S))
    {
        camera.transform->move(camera.transform->front_vector() * -current_speed, true);
    }

    if (Engine.KeyboardEvent.pressed(Engine.Key.D))
    {
        camera.transform->move(camera.transform->right_vector() * current_speed, true);
    }

    if (Engine.KeyboardEvent.pressed(Engine.Key.A))
    {
        camera.transform->move(camera.transform->right_vector() * -current_speed, true);
    }

    if (Engine.KeyboardEvent.pressed(Engine.Key.Space))
    {
        local k = (Engine.KeyboardEvent.pressed(Engine.Key.LeftShift) ? -1 : 1);
        camera.transform->move(camera.transform->up_vector() * current_speed * k, true);
    }

    if (Engine.KeyboardEvent.just_pressed(Engine.Key.Enter))
    {
        Engine.MouseEvent.relative_mode(!Engine.MouseEvent.relative_mode());
    }

    if (Engine.MouseEvent.relative_mode())
    {
        local offset = Engine.MouseEvent.offset() / (Engine.Window.size() / 2.0);
        camera.transform->rotate(-offset.x, Engine.Constants.OY, true);
        camera.transform->rotate(offset.y, camera.transform->right_vector(), true);
    }

    if (frame % 30 == 0)
    {
        execute_time = (os.clock() - execute_time) * 1000.0;
        print('Script update time: '..execute_time..' ms');
    }
}


return {update = camera_update};
