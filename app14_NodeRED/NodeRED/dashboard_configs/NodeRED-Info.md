Nó: *mqtt in*
    - Server: EMQx Broker
    - Tópico: FIAPIoT/sala1
    - QoS: 1


Function: *separarDadosSensores*
    - Setup:
        - Outputs: 5

    - On Message:
//Número de saídas: 5

let temp = { payload: msg.payload.temp };
let umid = { payload: msg.payload.umid };
let ic   = { payload: msg.payload.ic };
let dist = { payload: msg.payload.dist};
let mov  = { payload: msg.payload.mov};

return [ temp, umid, ic, dist, mov ];


Function: *alerta distância*
// Distância
msg.payload = {
    type: "message",
    chatId: "6239670426",
    content: `ALERTA: Objeto detectado a ${msg.payload} cm!`
}

return msg;



Function: *Alerta Presença*
// Movimento (booleano) - com validação
const mov = msg.payload;

// Verifica se é realmente um boolean
if (typeof mov !== 'boolean') {
    node.warn("Payload não é boolean: " + typeof mov);
    return null;
}

if (mov === true) {
    msg.payload = {
        type: "message",
        chatId: "6239670426",
        content: "ALERTA: Presença detectada!"
    };
    return msg;
}

return null;