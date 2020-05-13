const io = require('socket.io')(3000);

io
// Comment if you are not using an endpoint
.of('/user')
.on('connection', (socket) => {
	console.log("Client connected !");
	socket.on('message', (data) => { console.log("data: ", data); });
	socket.on('user-profile-info', (data) => { console.log("user-profile-info: ", data); });
	socket.on('disconnect', () => { });
	socket.emit("notification", { nb: 1, messages: [ "New notification :)" ] });
});
