
const WebSocket = require('ws');

// 创建 WebSocket 服务器，监听端口 8080
const server = new WebSocket.Server({ port: 8080 });

console.log('WebSocket server is starting on ws://localhost:8080');

// 当有客户端连接时
server.on('connection', (ws) => {
    console.log('Client connected');

    // 当接收到客户端消息时
    ws.on('message', (message) => {
        const parsedMessage = JSON.parse(message);
        console.log(`Received: ${parsedMessage.sender}: ${parsedMessage.text}`);

        // 向所有客户端广播消息
        server.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(JSON.stringify({
                    sender: parsedMessage.sender,
                    text: parsedMessage.text
                }));
            }
        });
    });

    // 当客户端断开连接时
    ws.on('close', () => {
        console.log('Client disconnected');
    });
});

console.log('WebSocket server is running');