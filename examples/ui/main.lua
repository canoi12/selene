local Rect = require('Rect')
local App = require('App')
--- @type App
local app = App.defaultEngine()
app:initUI('pastel-pop16')
local text = 'olar'
local text_list = {}
local rr = Rect.create(256, 64, 256, 96)
function app:draw(r)
    r:clearColor({ 0, 0, 0, 255 })
    r:clear()
    if app.ui:window('window1', 'Window', rr) then
        app.ui:label('label1', text)
        if app.ui:button('button1', 'Button') then
            table.insert(text_list, "btn1")
        end
        app.ui:breakLine()
        app.ui:button('btntst', 'Button')
    end
    if app.ui:window('window2', 'Window2') then
        app.ui:label('label2', 'Testandooo')
        app.ui:checkbox('checkbox')
        app.ui:breakLine()
        app.ui:label('conf', 'Enable VSync')
        app.ui:checkbox('checkbox2')
        app.ui:breakLine()
        app.ui:checkbox('checkbox3')
        app.ui:breakLine()
        app.ui:checkbox('checkbox1')
    end
    if app.ui:window('window3', 'Window3') then
        app.ui:label('label3', 'Teste')
        app.ui:checkbox('chk')
        if app.ui:button('btn3', 'Button') then
            table.insert(text_list, "btn3")
        end
    end

    local y = 0
    for i, c in pairs(app.ui.components) do
        r:print(i .. ': ' .. c.state, 0, y)
        y = y + 10
    end
end

function App:onEvent(ev)
    if ev.name == 'text input' then
        text = text .. ev.text.text
    end
end

return app
