class Transition:
    def __init__(self, source, symbol, target):
        self.source = source
        self.symbol = symbol
        self.target = target

    def __repr__(self):
        return f"Transition({self.source} --{self.symbol}--> {self.target})"
