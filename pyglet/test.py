import pyglet

window = pyglet.window.Window()
label = pyglet.text.Label("hello")


@window.event
def on_draw():
    window.clear()
    label.draw()


pyglet.app.run()
