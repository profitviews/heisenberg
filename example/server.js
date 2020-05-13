const io = require('socket.io')(3000);

io
// Comment if you are not using an endpoint
.of('/user')
.on('connection', (socket) => {
	console.log("Client connected !");

	socket.on('message', (data) => { console.log("data: ", data); });
	socket.on('receive-profile-info', (data) => { console.log("receive-profile-info: ", data); });
	socket.on('Emit to server', (data) => { console.log("Emit to server: ", data); });

	socket.emit("notification", { nb: 1, messages: [ "New notification :)" ] });
	// Send data to client (ex: request user info)
	socket.send("send-user-profile");
	// Emit data to client
	socket.emit("send-user-profile", {});

	// Disconnect
	socket.on('disconnect', () => { });
});
