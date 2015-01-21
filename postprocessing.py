def promedio(l):
	return reduce(lambda q,p: float(p)+float(q), l)/len(l)

filename = "Results.csv"
archivo = open(filename, 'r')
finales = []

for line in archivo:
	parciales = []
	datos = line.split(";")
	nsockets = int(datos[0])
	threads = range(1, len(datos))

	parciales.append(nsockets)
	for i in threads:
		muestras = datos[i].split(",")
		muestras = filter(lambda x: x!=" ", muestras)
		if(len(muestras)>1):
			parciales.append(promedio(muestras))
	finales.append(parciales)

salida = open("postProcessed.csv", "w+")
salida.write(";")
for j in threads:
	salida.write(str(j)+",")
salida.write("\n")
for l in finales:
	for i in l:
		salida.write(str(i)+",")
	salida.write("\n")