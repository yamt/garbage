import torch
from torch import nn
from torch import optim
import torch.nn.functional as F

import numpy as np


class Network(nn.Module):
    def __init__(self, sizes):
        super(Network, self).__init__()
        self.linear1 = nn.Linear(sizes[0], sizes[1])
        self.linear2 = nn.Linear(sizes[1], sizes[2])

    def forward(self, x):
        x = self.linear1(x)
        x = F.sigmoid(x)
        x = self.linear2(x)
        # revisit: pytorch seems to recommend to combine
        # the last actiaviton function with the cost function
        x = F.sigmoid(x)
        return x


def feed_forward(n, a):
    return n(torch.from_numpy(a)).detach().numpy()


def test(n, data, answers):
    assert len(data) == len(answers)
    data = torch.from_numpy(data)
    answers = torch.from_numpy(answers)
    n.eval()
    with torch.no_grad():
        r = n(data)
        r = torch.argmax(r, axis=1)
        return torch.sum(r == answers)


def sgd(n, data, answers, rate):
    answers = torch.from_numpy(answers)
    answers.requires_grad = True
    data = torch.from_numpy(data)
    data.requires_grad = True
    o = optim.SGD(n.parameters(), lr=rate)
    o.zero_grad()
    n.train()
    r = n(data)
    loss = F.mse_loss(r, answers)
    loss.backward()
    o.step()


print(f"setting torch threads {torch.get_num_threads()} -> 1")
torch.set_num_threads(1)
