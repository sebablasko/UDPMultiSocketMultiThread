#!/bin/bash

salida=Results.csv
MAX_PACKS=10000000
repetitions=10

echo "Compilando..."
make all
echo "Done"

echo "Ejecutando Prueba..."

#Definir aquí el límite de sockets
for num_sockets in {1..16}
do
	linea=$num_sockets";"

	#Definir aqui el límite de Threads
	for ((num_threads=1 ; $num_threads<=32 ; num_threads++))
	{
		echo "Evaluando "$num_threads" Threads y con "$num_sockets" Sockets"
		for ((i=1 ; $i<=$repetitions ; i++))
		{
			if(($num_threads < $num_sockets))
			then
				echo "No se puede Evaluar"
				linea=$linea"0, "
			else
				echo "Repeticion "$i
				./server $MAX_PACKS $num_threads $num_sockets > aux &
				pid=$!
				sleep 1

				./client $(($MAX_PACKS*10)) $num_sockets 127.0.0.1 > /dev/null &
				./client $(($MAX_PACKS*10)) $num_sockets 127.0.0.1 > /dev/null &
				./client $(($MAX_PACKS*10)) $num_sockets 127.0.0.1 > /dev/null &
				./client $(($MAX_PACKS*10)) $num_sockets 127.0.0.1 > /dev/null &
				wait $pid
				linea="$linea$(cat aux)"
				rm aux
			fi
		}
		linea="$linea;"
		echo ""
	}
	echo "$linea" >> $salida
done
make clean
echo "Done"