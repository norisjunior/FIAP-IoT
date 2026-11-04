# Roteiro: ouvir a sua wake word pelo microfone do notebook

São **2 terminais**, todos abertos na pasta `notebook`. Não feche nenhum.

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
docker run --rm -it -p 1337:1337 public.ecr.aws/g7a8t7v6/inference-container:v1.95.15 --api-key SUA_CHAVE --run-http-server 1337
```

Espere aparecer `HTTP Server now running`.

## 2. Terminal 2: ouvir

Só na primeira vez:

```
py -3.12 -m venv venv
.\venv\Scripts\python.exe -m pip install -r requirements.txt
```

Depois:

```
.\venv\Scripts\python.exe wakeword.py
```

Fale a sua wake word perto do microfone.

## 3. Terminar

`Ctrl+C` nos 2 terminais.
