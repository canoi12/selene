local App = require('App')
--- @type App
local app = App.defaultEngine()
app:initUI('pastel-pop16')
local text = 'olar'
local text_list = {}
function app:draw(r)
    r:clearColor({ 0, 0, 0, 255 })
    r:clear()
    if app.ui:window('window1', 'Window') then
        app.ui:label('label1', text)
        if app.ui:button('button1', 'Button') then
            table.insert(text_list, "btn1")
        end
    end
    if app.ui:window('window2', 'Window2') then
        app.ui:label('label2', 'Testandooooooooooo')
    end
    if app.ui:window('window3', 'Window3') then
        app.ui:label('label3', 'Teste')
        if app.ui:button('btn3', 'Button') then
            table.insert(text_list, "btn3")
        end
    end

    local y = 0
    for i, t in ipairs(text_list) do
        r:print(t, 0, y)
        y = y + 10
    end
end

function App:onEvent(ev)
    if ev.name == 'text input' then
        text = text .. ev.text.text
    end
end

return app
