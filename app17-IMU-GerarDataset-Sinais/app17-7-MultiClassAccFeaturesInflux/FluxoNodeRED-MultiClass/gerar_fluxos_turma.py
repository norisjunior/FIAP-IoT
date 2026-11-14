# gerar_fluxos_turma.py: um fluxo do Node-RED por grupo, para o Node-RED do professor.
#
# A turma inteira publica no mesmo broker e no mesmo topico. No Node-RED do professor,
# cada grupo ganha uma aba igual ao flow_multiclasse_influx.json, com MEU_DEVICE = o nome
# da placa do grupo e o InfluxDB do grupo (URL, org, bucket, token que o grupo entregou).
#
# Uso, nesta pasta:
#   python gerar_fluxos_turma.py turma.csv            -> escreve flows_turma.json
#   python gerar_fluxos_turma.py turma.csv --enviar http://localhost:1880 usuario senha
#                                                     -> tambem envia para o Node-RED (API admin)
#
# turma.csv (uma linha por grupo; cabecalho obrigatorio):
#   grupo,device,url,org,bucket,token
#   03,grupo03,https://us-east-1-1.aws.cloud2.influxdata.com,abc123,IoTSensores,xxxxxxxx
#
# Importar o JSON pelo menu do Node-RED nao traz o token (o Node-RED nunca exporta
# credenciais): use --enviar, que grava pela API admin, ou cole o token de cada aba a mao.
import csv
import json
import os
import re
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
MODELO = os.path.join(AQUI, "flow_multiclasse_influx.json")
SAIDA = os.path.join(AQUI, "flows_turma.json")


def ler_turma(caminho):
    with open(caminho, encoding="utf-8-sig", newline="") as f:
        linhas = list(csv.DictReader(f))
    obrigatorias = ["grupo", "device", "url", "org", "bucket", "token"]
    for col in obrigatorias:
        if linhas and col not in linhas[0]:
            sys.exit(f"turma.csv: falta a coluna {col}")
    for l in linhas:
        for col in obrigatorias:
            l[col] = (l[col] or "").strip()
        if not re.fullmatch(r"[A-Za-z0-9_-]+", l["device"]):
            sys.exit(f"device invalido para o grupo {l['grupo']}: '{l['device']}' (so letras, numeros, _ e -)")
    return linhas


def aba_do_grupo(modelo, grupo):
    """Copia o fluxo-modelo com ids proprios, MEU_DEVICE e o InfluxDB do grupo."""
    sufixo = "_g" + re.sub(r"[^A-Za-z0-9]", "", grupo["grupo"])
    ids = {n["id"]: n["id"] + sufixo for n in modelo if n.get("type") != "mqtt-broker"}
    nos = []
    for n in modelo:
        if n.get("type") == "mqtt-broker":
            continue   # o broker da sala e um so: fica fora das abas, compartilhado
        n = json.loads(json.dumps(n))
        n["id"] = ids[n["id"]]
        if "z" in n:
            n["z"] = ids[n["z"]]
        if "wires" in n:
            n["wires"] = [[ids.get(w, w) for w in saida] for saida in n["wires"]]
        if n.get("type") == "tab":
            n["label"] = f"grupo {grupo['grupo']} - {grupo['device']} -> InfluxDB"
        elif n.get("type") == "function":
            n["func"] = n["func"].replace('const MEU_DEVICE = "";', f'const MEU_DEVICE = "{grupo["device"]}";', 1)
            assert grupo["device"] in n["func"]
        elif n.get("type") == "influxdb out":
            n["influxdb"] = ids[n["influxdb"]]
            n["org"] = grupo["org"]
            n["bucket"] = grupo["bucket"]
            n["name"] = f"InfluxDB do grupo {grupo['grupo']} (vibracao_multiclasse)"
        elif n.get("type") == "influxdb":
            n["url"] = grupo["url"]
            n["name"] = f"InfluxDB Cloud - grupo {grupo['grupo']}"
            n["credentials"] = {"token": grupo["token"]}   # so a API admin le isto
        nos.append(n)
    return nos


def gerar(turma):
    modelo = json.load(open(MODELO, encoding="utf-8"))
    broker = [n for n in modelo if n.get("type") == "mqtt-broker"]
    nos = list(broker)
    for g in turma:
        nos += aba_do_grupo(modelo, g)
    return nos


def enviar(nos, base, usuario, senha):
    """Acrescenta as abas aos fluxos que ja existem no Node-RED (nao apaga nada)."""
    import urllib.request

    def req(metodo, caminho, corpo=None, token=None, extra=None):
        dados = json.dumps(corpo).encode() if corpo is not None else None
        r = urllib.request.Request(base + caminho, data=dados, method=metodo)
        r.add_header("Content-Type", "application/json")
        if token:
            r.add_header("Authorization", "Bearer " + token)
        for k, v in (extra or {}).items():
            r.add_header(k, v)
        with urllib.request.urlopen(r) as resp:
            return json.loads(resp.read() or b"null")

    token = None
    if usuario:
        token = req("POST", "/auth/token", {"client_id": "node-red-admin", "grant_type": "password",
                                             "scope": "*", "username": usuario, "password": senha})["access_token"]
    atual = req("GET", "/flows", token=token, extra={"Node-RED-API-Version": "v2"})
    existentes = {n["id"] for n in atual["flows"]}
    novos = [n for n in nos if n["id"] not in existentes]
    if not novos:
        print("nada novo para enviar")
        return
    req("POST", "/flows", {"flows": atual["flows"] + novos, "rev": atual["rev"]}, token=token,
        extra={"Node-RED-API-Version": "v2", "Node-RED-Deployment-Type": "nodes"})
    print(f"enviados {len(novos)} nos para {base}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__ or "uso: python gerar_fluxos_turma.py turma.csv [--enviar URL usuario senha]")
    turma = ler_turma(sys.argv[1])
    nos = gerar(turma)
    with open(SAIDA, "w", encoding="utf-8") as f:
        json.dump(nos, f, indent=4, ensure_ascii=False)
    abas = [n["label"] for n in nos if n.get("type") == "tab"]
    print(f"{os.path.basename(SAIDA)}: {len(abas)} abas, {len(nos)} nos")
    for a in abas:
        print("  ", a)
    if "--enviar" in sys.argv:
        i = sys.argv.index("--enviar")
        base = sys.argv[i + 1].rstrip("/")
        usuario = sys.argv[i + 2] if len(sys.argv) > i + 2 else None
        senha = sys.argv[i + 3] if len(sys.argv) > i + 3 else None
        enviar(nos, base, usuario, senha)
