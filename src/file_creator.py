def cria_arquivo():
    filename = input('Type the name of the file:')
    out_file = open(filename, 'w+')
    n = int(input('Type the maximum of elements (multiple of 10):'))
    for i in range(0,n-1):
        out_file.write("um\n")
        out_file.write("dois\n")
        out_file.write("tres\n")
        out_file.write("quatro\n")
        out_file.write("cinco\n")
        out_file.write("seis\n")
        out_file.write("sete\n")
        out_file.write("oito\n")
        out_file.write("nove\n")
        out_file.write("dez\n")
    out_file.close()

cria_arquivo()