import threading
import time


def f():
    for i in range(5):
        print(f"f {i}")
        time.sleep(1)


def g():
    for i in range(5):
        print(f"g {i}")
        time.sleep(1)


print("creating threads")
tf = threading.Thread(target=f)
tg = threading.Thread(target=g)
print("starting threads")
tf.start()
tg.start()
print("joining threads")
tf.join()
tg.join()
print("done")
