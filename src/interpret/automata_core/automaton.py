from .state import State
from .transition import Transition

class Automaton:
    def __init__(self):
        self.states = []
        self.transitions = []

    def add_state(self, state: State):
        self.states.append(state)

    def add_transition(self, transition: Transition):
        self.transitions.append(transition)