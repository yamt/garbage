import threading

def produce():
	pass

def consume():
	pass

print("creating threads");
t1 = threading.Thread(target=produce)
t2 = threading.Thread(target=consume)

print("starting threads");
t1.start()
t2.start()

print("joining threads");
t1.join()
t2.join()

print("done");
