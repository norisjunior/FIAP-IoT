FROM nodered/node-red:latest-debian

USER root

# Toolchain + headers p/ módulos nativos (sqlite3)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential python3 python3-distutils libsqlite3-dev ccache \
 && rm -rf /var/lib/apt/lists/*

# Ajuda o node-gyp e permite instalar como root dentro do build
ENV npm_config_unsafe_perm=true \
    npm_config_python=/usr/bin/python3 \
    npm_config_jobs=2 \
    CC="ccache gcc" \
    CXX="ccache g++"

WORKDIR /usr/src/node-red

# Instala primeiro o "pesado" para aproveitar cache de camadas
RUN npm install --no-audit --no-fund --verbose \
    node-red-node-sqlite

# Depois o restante dos nós
RUN npm install --no-audit --no-fund --verbose \
    node-red-contrib-influxdb \
    node-red-contrib-dsm \
    node-red-contrib-cron-plus \
    node-red-contrib-ui-led \
    node-red-contrib-ui-level \
    node-red-node-email \
    node-red-node-random \
    node-red-contrib-telegrambot \
    node-red-contrib-cbor \
    node-red-dashboard

USER node-red