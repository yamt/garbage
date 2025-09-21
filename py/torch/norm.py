import torch
from torch.utils.data import DataLoader
from torchvision import datasets, transforms


def mean_std(data, ch):
    sum = torch.zeros(ch)
    ssum = torch.zeros(ch)
    total = 0

    loader = DataLoader(data, batch_size=64, shuffle=False)
    for b, _ in loader:
        (n, c, h, w) = b.shape
        b = b.view(n, c, -1)
        sum += b.sum(2).sum(0)
        ssum += (b**2).sum(2).sum(0)
        total += n * h * w

    mean = sum / total
    std = torch.sqrt(ssum / total - mean**2)
    return mean, std


ch = 1

train_data = datasets.MNIST(
    "data",
    train=True,
    download=True,
    transform=transforms.ToTensor(),
)

mean, std = mean_std(train_data, ch)
print(f"mean {mean} std {std}")

transform = transforms.Compose(
    [transforms.ToTensor(), transforms.Normalize(mean.tolist(), std.tolist())]
)
train_data_n = datasets.MNIST(
    "data",
    train=True,
    download=True,
    transform=transform,
)

mean, std = mean_std(train_data_n, ch)
print(f"mean {mean} std {std}")
