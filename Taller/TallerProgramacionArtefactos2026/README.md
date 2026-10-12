# TallerProgramacionArtefactos2026

## Dashboard de clima con ESP32, MQTT y React

Este proyecto muestra en React los datos enviados por una ESP32 mediante un broker Mosquitto. Mosquitto se ejecuta dentro de Docker y React se conecta por WebSocket.

### Arquitectura

```text
ESP32 -- MQTT:1883 --> Mosquitto en Docker -- WebSocket:9001 --> Dashboard React
```

La aplicación recibe tres topics independientes:

| Dato | Topic |
| --- | --- |
| Temperatura | `clima/esp32/temperatura` |
| Humedad | `clima/esp32/humedad` |
| Luz solar | `clima/esp32/luz` |

La gráfica conserva las últimas cinco temperaturas recibidas. La condición del clima depende de la luz solar:

- Menos de `20`: lluvioso.
- De `20` a `64`: parcialmente nublado.
- Desde `65`: soleado.

## 1. Requisitos

Instalar en todos los equipos:

- Git.
- Docker Desktop, o Docker Engine y Docker Compose en Linux.
- Node.js LTS y npm.
- Arduino IDE o PlatformIO para programar la ESP32.

Comprobar Node.js y npm:

```bash
node --version
npm --version
```

Comprobar Docker:

```bash
docker --version
docker compose version
```

El segundo comando debe responder con una versión de Docker Compose v2.

## 2. Configuración en Linux Ubuntu/Debian

### Instalar Docker

Si Docker todavía no está instalado:

```bash
sudo apt update
sudo apt install docker.io docker-compose-v2
```

Iniciar Docker y configurarlo para que arranque con el sistema:

```bash
sudo systemctl enable --now docker
```

Si el comando `docker` requiere `sudo`, agregar el usuario actual al grupo Docker:

```bash
sudo usermod -aG docker "$USER"
```

Cerrar sesión y volver a entrar para que el cambio tenga efecto. Después comprobar:

```bash
docker run --rm hello-world
docker compose version
```

En algunas versiones de Ubuntu el paquete se llama `docker-compose-v2`; no usar `docker-compose-plugin` si no aparece en los repositorios de esa distribución.

## 3. Configuración en Windows

1. Instalar Docker Desktop desde https://www.docker.com/products/docker-desktop/.
2. Durante la instalación, mantener habilitado el backend WSL 2 si Docker lo solicita.
3. Reiniciar Windows si el instalador lo solicita.
4. Abrir Docker Desktop y esperar a que indique que Docker está ejecutándose.
5. Abrir PowerShell y comprobar:

```powershell
docker --version
docker compose version
```

Si Windows Firewall pregunta si Docker puede comunicarse en la red, permitirlo en la red privada. La ESP32 y la computadora deben estar conectadas a la misma red local.

## 4. Descargar y preparar el proyecto

Desde una terminal, entrar en la carpeta del proyecto:

### Linux

```bash
cd ~/Desktop/TallerProgramacionArtefactos2026
```

### Windows PowerShell

```powershell
cd "$HOME\Desktop\TallerProgramacionArtefactos2026"
```

Instalar las dependencias de React:

```bash
npm install
```

Crear el archivo de variables de entorno:

### Linux

```bash
cp .env.example .env
```

### Windows PowerShell

```powershell
Copy-Item .env.example .env
```

La configuración por defecto usa:

```text
VITE_MQTT_URL=ws://localhost:9001
VITE_MQTT_TEMPERATURE_TOPIC=clima/esp32/temperatura
VITE_MQTT_HUMIDITY_TOPIC=clima/esp32/humedad
VITE_MQTT_SUNLIGHT_TOPIC=clima/esp32/luz
```

## 5. Iniciar Mosquitto con Docker

Importante: estos comandos deben ejecutarse desde la raíz del proyecto, donde existe `docker-compose.yml`:

### Linux

```bash
cd ~/Desktop/TallerProgramacionArtefactos2026
```

### Windows PowerShell

```powershell
cd "$HOME\Desktop\TallerProgramacionArtefactos2026"
```

Desde la raíz del proyecto, ejecutar:

```bash
docker compose up -d
```

Este comando descarga la imagen de Mosquitto, aplica [config/mosquitto.conf](config/mosquitto.conf) y publica:

- `1883`: conexión MQTT de la ESP32.
- `9001`: conexión WebSocket del navegador.

Comprobar el contenedor:

```bash
docker compose ps
docker compose logs mosquitto
```

Debe aparecer el contenedor `clima-mosquitto` en estado `Up`.

Para detenerlo:

```bash
docker compose down
```

Los datos del broker se conservan en volúmenes Docker. Para detenerlo y borrar también esos datos:

```bash
docker compose down -v
```

## 6. Iniciar React

En otra terminal, dentro de la carpeta del proyecto:

```bash
npm run dev
```

Abrir en el navegador:

```text
http://localhost:5173
```

Si el navegador está en otra computadora, iniciar Vite con:

```bash
npm run dev -- --host 0.0.0.0
```

En ese caso, usar en `.env` la IP de la computadora que ejecuta Mosquitto:

```text
VITE_MQTT_URL=ws://IP_DE_LA_COMPUTADORA:9001
```

Después de modificar `.env`, reiniciar `npm run dev`.

## 7. Configurar la ESP32

La ESP32 debe conectarse a la misma red local que la computadora y publicar en la IP de esa computadora:

```text
Servidor MQTT: IP_DE_LA_COMPUTADORA
Puerto: 1883
Usuario: ninguno, solo para este taller
Contraseña: ninguna, solo para este taller
```

Ejemplos de publicaciones:

```text
Topic: clima/esp32/temperatura
Mensaje: 24.5

Topic: clima/esp32/humedad
Mensaje: 58

Topic: clima/esp32/luz
Mensaje: 74
```

También se acepta un JSON por topic, por ejemplo:

```json
{
	"temperatura": 24.5
}
```

## 8. Probar sin ESP32

Para probar la interfaz manualmente, publicar mensajes desde el contenedor de Mosquitto:

```bash
docker compose exec mosquitto mosquitto_pub -h localhost -t clima/esp32/temperatura -m 26 -r
docker compose exec mosquitto mosquitto_pub -h localhost -t clima/esp32/humedad -m 61 -r
docker compose exec mosquitto mosquitto_pub -h localhost -t clima/esp32/luz -m 12 -r
```

El dashboard debe mostrar `26°`, `61%` y el estado `Lluvioso`. Publicar otra temperatura agrega un punto a la gráfica y conserva solo las cinco más recientes.

La opción `-r` guarda el último valor en Mosquitto. Así, si React se conecta después de publicar o se recarga el navegador, recibe inmediatamente la última medición.

## 9. Problemas frecuentes

### `docker compose: command not found`

En Linux instalar Docker Compose v2:

```bash
sudo apt update
sudo apt install docker-compose-v2
```

En Windows abrir Docker Desktop antes de ejecutar los comandos.

### El contenedor no inicia

Revisar el error completo:

```bash
docker compose logs mosquitto
```

Confirmar que los puertos `1883` y `9001` no estén ocupados por otro programa.

### React muestra `Sin conexion`

Confirmar que Mosquitto está activo con `docker compose ps` y que el comando se ejecutó desde la raíz del proyecto. En la consola del navegador deben aparecer los mensajes `[MQTT] Conectado a` y `[MQTT] Suscrito a`. Si React se ejecuta desde otra computadora, no usar `localhost`: reemplazarlo por la IP del equipo donde corre Docker en `VITE_MQTT_URL`.

### La ESP32 no conecta

Confirmar que usa la IP local correcta de la computadora, que está en la misma red WiFi y que el firewall permite el puerto TCP `1883`.

### Seguridad

La configuración incluida permite conexiones anónimas para facilitar el taller. No usarla directamente en producción. En un despliegue real se deben configurar usuario, contraseña, ACL y TLS.

## Dashboard MQTT

La aplicación escucha tres topics mediante WebSocket en `ws://localhost:9001`:

- `clima/esp32/temperatura`
- `clima/esp32/humedad`
- `clima/esp32/luz`

### Inicio con Docker Compose

Solo es necesario instalar Docker Desktop o Docker Engine con Compose y ejecutar desde esta carpeta:

```bash
docker compose up -d
```

El comando descarga Mosquitto automáticamente, aplica [config/mosquitto.conf](config/mosquitto.conf) y deja disponibles los puertos `1883` y `9001`. Para detenerlo:

```bash
docker compose down
```

Después, copia `.env.example` como `.env` si necesitas cambiar la dirección del broker y ejecuta `npm run dev` para iniciar React.

Cada topic puede recibir un número directo, por ejemplo `24.5`, o un JSON con el nombre de su sensor:

```json
{
	"temperatura": 24.5
}
```

La gráfica conserva las últimas cinco temperaturas recibidas. La condición cambia según la luz solar: menos de 20 es lluvioso, de 20 a 64 es parcialmente nublado y desde 65 es soleado. La ESP32 debe conectarse a la IP de la computadora donde corre Docker, usando el puerto `1883`; React usa el WebSocket `ws://localhost:9001`. La configuración incluida permite conexiones anónimas solo para el taller y desarrollo local; para producción se deben añadir usuario, contraseña y TLS.