var targetUrl = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

function onLoad() {
    initializeSocket();
    setupControls();
}

function initializeSocket() {
    console.log('Opening WebSocket connection to ESP32...');
    websocket = new WebSocket(targetUrl);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('WebSocket connection opened');
}

function onClose(event) {
    console.log('WebSocket connection closed');
    setTimeout(initializeSocket, 2000);
}

function onMessage(event) {
    console.log("Raw WebSocket message received: '", event.data, "'");
}

function sendMessage(message) {
    if (websocket.readyState === WebSocket.OPEN) {
        var jsonMessage = JSON.stringify({ command: message });
        console.log('Sending message:', jsonMessage);
        websocket.send(jsonMessage);
    } else {
        console.log('WebSocket connection is not open');
    }
}

function setupControls() {
    var buttons = document.querySelectorAll('.joystick button');

    buttons.forEach(function(button) {
        button.addEventListener('touchstart', function() {
            console.log('Button touched:', button.id);
            sendMessage(button.id);
        });

        button.addEventListener('touchend', function() {
            console.log('Button released:', button.id);
            setTimeout(() => {
                sendMessage('stop');
            }, 200);
        });
    });

    var automodeButton = document.getElementById('automode');
    automodeButton.addEventListener('click', function() {
        if (automodeButton.classList.contains('latched')) {
            automodeButton.classList.remove('latched');
            sendMessage('automode_off');
        } else {
            automodeButton.classList.add('latched');
            sendMessage('automode_on');
        }
    });
}
