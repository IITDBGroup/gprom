
import sys

if __name__ == '__main__':
    rv = []
    for i in range(50):
        if i <= 40:
            rv.append(i * 5)
        else:
            rv.append(rv[-1] + 200)

    # print(rv)


    print(sys.path)