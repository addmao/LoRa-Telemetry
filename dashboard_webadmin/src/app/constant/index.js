module.exports = {
    api : {
        //HOST: 'http://192.168.43.187:9001',//Raspberry Pi Zero w
        HOST: 'http://192.168.43.194:9001',//Raspberry Pi 3 B+
        // HOST: 'http://localhost:3000/',
        TEST_URL: "../assets/data/flow.json",
        TEST_WEBSTAT_URL: "../assets/data/web.json",
        FLOW_URL: "flowstat/",
        WEB_URL: "webstat/",
    },
    remoteHostname : 'http://centraldesk-api.maxile.net/remote?',
    local : {
        token: 'projectUser',
        user: 'projectUserInfo'
    }
    
}
