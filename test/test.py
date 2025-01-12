

# import megastructure as mega

from statemachine import StateMachine, State

class ShortcutMachine(StateMachine):
    "Ctrl Tab Shortcut Machine"

    start = State(initial=True)
    tabbing = State()
    
    KEY_LEFTCTRL_DOWN = ( 
        start.to(tabbing)
    )

    KEY_LEFTCTRL_UP = (
        tabbing.to(start)
    )

    KEY_TAB_DOWN = (
        tabbing.to(tabbing)
    )

    def on_enter_start(self):
        print("start")

    def on_enter_tabbing(self):
        print("tabbing: iterate through MRU here")

    def on_exit_tabbing(self):
        print( "exiting tabbing: update MRU here" )


sm = ShortcutMachine()

sm._graph().write_png("test.png")


def onEvent( key, down ):

    event = ""
    if down:
        event = key + "_DOWN"
    else:
        event = key + "_UP"
    print( "Received event: " + event )

    if event in sm.events:
        sm.send(event)

    return False


