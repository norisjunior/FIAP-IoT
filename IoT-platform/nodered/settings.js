// Os valores são fornecidos pelo Compose a partir do .env.
module.exports = {
    adminAuth: {
        type: "credentials",
        users: [{
            username: process.env.NODERED_USER,
            password: process.env.NODERED_PASSWORD_HASH,
            permissions: "*"
        }]
    }
};
