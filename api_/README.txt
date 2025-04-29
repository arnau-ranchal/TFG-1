Per executar:
1. Engegar el Docker Desktop
2. Obrir terminal al projecte i fer: docker build -t tfg_api .
3. Ara per executar en local: docker run -p 8000:8000 tfg_api  
4. Obrir navegador i anar a localhost:8000
5.(OPCIONAL) Pel Swagger de FastApi anar a localhost:8000/docs
6. Al executar les funcions desde el navegador pots veure la seva resposta al POST/GET (dels Endpoints) a la terminal
7. Un cop acabes de jugar amb la API, ves a la terminal on has executat el projecte i fes ctrl+C
8. Ara para i neteja els teus contenidors en la mateixa terminal amb docker rm -f $(docker ps -aq) 

COMANDOS:

# Si es modifiquen les funcions c++:
make clean && make	# Encara que això suposadament ja ho fa el Dockerfile

# Construeix container
docker build -t project_api .

# Executar container
docker run -p 8000:8000 project_api

Al navegador:
http://localhost:8000/docs

CONTAINER MANAGEMENT:

docker ps # Llista dels containers en execució
docker stop <container_id> # Per parar
docker rm <container_id> # Per eliminar

docker stop $(docker ps -q) # Parar tots els containers a l'hora
docker rm $(docker ps -aq) # Eliminar tots els containers detinguts

docker rm -f $(docker ps -aq) # Eliminar tot alhora ja estiguin els containers en execució o detinguts
docker system prune -a # Netejar containers i imatges en desús