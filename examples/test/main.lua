local i = 1
-- error('eita')
selene.set_step(function()
    print('Iteration', i)
    if i >= 50 then
        selene.set_running(false)
    end
    i = i + 1
end)
selene.set_quit(function()
    print('Quitting')
end)