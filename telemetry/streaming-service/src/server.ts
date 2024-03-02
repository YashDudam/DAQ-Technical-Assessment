import net from "net";
import { WebSocket, WebSocketServer } from "ws";

interface VehicleData {
  battery_temperature: number;
  timestamp: number;
}

const TCP_PORT = 12000;
const WS_PORT = 8080;
const tcpServer = net.createServer();
const websocketServer = new WebSocketServer({ port: WS_PORT });

tcpServer.on("connection", (socket) => {
  console.log("TCP client connected");

  let tempExceededCount = 0;
  let lastTempExceededTime = 0;
  socket.on("data", (msg) => {
    console.log(`Received: ${msg.toString()}`);

    if (msg.toString().endsWith('}}')) return;

    const jsonData: VehicleData = JSON.parse(msg.toString());

    if (jsonData.battery_temperature < 20 || jsonData.battery_temperature > 80) {

      const currentTimestamp = jsonData.timestamp;

      // Check if it's been more than 5 seconds since the last out-of-range event
      if (currentTimestamp - lastTempExceededTime > 5000) {
        // Reset the count if it's been more than 5 seconds
        tempExceededCount = 1;
      } else {
        // Increment the count if it's within the 5-second window
        tempExceededCount++;
      }

      // Update the timestamp of the last out-of-range event
      lastTempExceededTime = currentTimestamp;

      // Check if the temperature has exceeded the range more than 3 times in 5 seconds
      if (tempExceededCount > 3) {
        console.log(`Timestamp: ${currentTimestamp}, Battery temperature out of safe range`);
      }
    }


    // Send JSON over WS to frontend clients
    websocketServer.clients.forEach(function each(client) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(msg.toString());
      }
    });
  });

  socket.on("end", () => {
    console.log("Closing connection with the TCP client");
  });

  socket.on("error", (err) => {
    console.log("TCP client error: ", err);
  });
});

websocketServer.on("listening", () =>
  console.log(`Websocket server started on port ${WS_PORT}`)
);

websocketServer.on("connection", async (ws: WebSocket) => {
  console.log("Frontend websocket client connected");
  ws.on("error", console.error);
});

tcpServer.listen(TCP_PORT, () => {
  console.log(`TCP server listening on port ${TCP_PORT}`);
});
