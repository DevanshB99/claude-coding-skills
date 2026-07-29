class Account:
    def __init__(self, owner, num, bal, pwd, bank, branch, currency, overdraft):
        self.owner = owner
        self.num = num
        self.bal = bal
        self.pwd = pwd
        self.bank = bank
        self.branch = branch
        self.currency = currency
        self.overdraft = overdraft
        self.log = []

    def do(self, action, amt, pwd, notify=False, fee=True):
        if pwd != self.pwd:
            return -1
        if action == "w":
            if self.bal - amt < -self.overdraft:
                return -2
            self.bal = self.bal - amt
            if fee: self.bal = self.bal - 0.5
        elif action == "d":
            self.bal = self.bal + amt
        else:
            return -3
        self.log.append(action + str(amt))
        if notify:
            print("Dear " + self.owner + ", your balance is " + str(self.bal))
        return self.bal
