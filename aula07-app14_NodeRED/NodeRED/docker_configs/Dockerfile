FROM nodered/node-red:latest

USER root

RUN npm install \
    node-red-dashboard \
    node-red-contrib-ui-led \
    node-red-contrib-ui-level \
    node-red-contrib-telegrambot \
    node-red-contrib-influxdb \
    node-red-node-sqlite \
    node-red-contrib-dsm \
    node-red-contrib-cron-plus \
    node-red-node-email \
    node-red-node-random

USER node-red