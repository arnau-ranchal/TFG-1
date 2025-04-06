# Si es modifiquen les funcions c++:
make clean && make

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
