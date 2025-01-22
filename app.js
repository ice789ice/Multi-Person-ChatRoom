// 连接到 WebSocket 服务器
const socket = new WebSocket('ws://localhost:8080');

const messagesDiv = document.getElementById('messages');
const messageInput = document.getElementById('message-input');
const sendButton = document.getElementById('send-button');

// 将消息添加到聊天框
function appendMessage(message) {
    const messageElement = document.createElement('div');
    messageElement.textContent = message;
    messagesDiv.appendChild(messageElement);
}

// 当按钮被点击时发送消息
sendButton.addEventListener('click', () => {
    const message = messageInput.value;
    socket.send(message);
    messageInput.value = ''; // 清空输入框
});

// 接收来自服务器的消息
socket.addEventListener('message', event => {
    appendMessage(event.data);
});

// 处理 WebSocket 连接打开事件
socket.addEventListener('open', () => {
    appendMessage('Connected to the server.');
});

// 处理 WebSocket 连接关闭事件
socket.addEventListener('close', () => {
    appendMessage('Disconnected from the server.');
});