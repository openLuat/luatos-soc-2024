import sys

def main():
    if not sys.argv[1]:
        print("bin file error")
        return
    binListData = []
    file = open(sys.argv[1], 'rb')
    file.seek(0, 0)
    while True:
        binByte = file.read(1)
        if len(binByte) == 0:
            break
        else:
            binListData.append("0x%.2x" % ord(binByte))
    file.close()
    fileOutput = open("bin2c.c", 'w')
    fileOutput.write("unsigned long hexLength = {};\n".format(len(binListData)))
    fileOutput.write("unsigned char hexData[] = \n")
    fileOutput.write("{\n")
    for i in range(len(binListData)):
        if (i != 0) and (i % 16 == 0):
            fileOutput.write("\n")
            fileOutput.write(binListData[i] + ",")
        elif (i + 1) == len(binListData):
            fileOutput.write(binListData[i])
        else:
            fileOutput.write(binListData[i] + ",")
    fileOutput.write("\n};")
    fileOutput.close()
    print("bin file to C array file success!!!")

if __name__ == '__main__':
    main()