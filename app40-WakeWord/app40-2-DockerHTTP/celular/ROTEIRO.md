# Roteiro: ouvir a sua wake word pelo celular

São **3 terminais**, todos abertos na pasta `celular`. Não feche nenhum.

## 0. Instalar o Python 3.12 (só uma vez)

```
winget install -e --id Python.Python.3.12 --scope user
```

Feche e abra o terminal. Confira:

```
py -3.12 --version
```

Crie o venv com `py -3.12`: o `python` sozinho pode abrir outra versão, como a 3.14. Depois, o `.\venv\Scripts\python.exe` já é o 3.12.

## 1. Terminal 1: subir o modelo

No Edge Impulse, vá em **Deployment** e escolha **Docker container**. Copie o comando
*"in a one-liner locally"* e cole **numa linha só**.

```
docker run --rm -it -p 1337:1337 public.ecr.aws/g7a8t7v6/inference-container:v1.95.15 --api-key ei_6b34e0fce172c23c8bed39597a7e005fcc8895a8c0f59b56 --run-http-server 1337
```

Espere aparecer `HTTP Server now running`.

## 2. Terminal 2: subir o servidor

Só na primeira vez:

```
py -3.12 -m venv venv
.\venv\Scripts\python.exe -m pip install -r requirements.txt
```

Depois:

```
.\venv\Scripts\python.exe -m uvicorn servidor:app --port 8000
```

## 3. Terminal 3: abrir o túnel

```
docker run --rm -it cloudflare/cloudflared tunnel --url http://host.docker.internal:8000
```

Copie o endereço que termina em `.trycloudflare.com`.

## 4. No celular

Abra o endereço, toque em **Ouvir**, permita o microfone e fale a sua wake word.

## 5. Terminar

`Ctrl+C` nos 3 terminais.
