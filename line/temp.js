const express = require('express');
const bodyParser = require('body-parser');
const crypto = require('crypto');

const app = express();
const PORT = 3000;

// Your LINE channel secret
const CHANNEL_SECRET = '773ad4b071151d20c00955bbba1caea7';

const { initializeApp } = require("firebase/app");
const { getDatabase, ref, set } = require("firebase/database");
const { stat } = require('fs');

// Your Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyAld8mAMkx-3bGBbOAS0-eqTn4QX7PyGn0",
  authDomain: "es-project-f6e5f.firebaseapp.com",
  databaseURL: "https://es-project-f6e5f-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "es-project-f6e5f",
  storageBucket: "es-project-f6e5f.firebasestorage.app",
  messagingSenderId: "73640823998",
  appId: "1:73640823998:web:d6e340e9f63bbae287e2ef"
};

// Initialize Firebase
const firebaseApp = initializeApp(firebaseConfig);
const database = getDatabase(firebaseApp);

// Parse JSON requests
app.use(bodyParser.json());

// Verify the request signature
const verifySignature = (req, res, next) => {
  const signature = req.headers['x-line-signature'];
  const body = JSON.stringify(req.body);
  const hash = crypto.createHmac('sha256', CHANNEL_SECRET).update(body).digest('base64');

  if (hash === signature) {
    next(); // Proceed to the next middleware
  } else {
    res.status(403).send('Invalid signature');
  }
};

// Webhook endpoint
app.post('/webhook', verifySignature, (req, res) => {
  console.log('Webhook received:', req.body);

  req.body.events.forEach((event) => {

    // Example: Update Firebase when receiving a text message
    if (event.type === 'message' && event.message.type === 'text') {
      const messageText = event.message.text; // User's message

      if (messageText == "@water") {
      // Reference a location in the database (e.g., `messages/{userId}`)
      const dbRef = ref(database, `water`);

      // Save the message to Firebase
      set(dbRef, {
        status: 1,
      })
        .then(() => {
          console.log("Message saved to Firebase.");
        })
        .catch((error) => {
          console.error("Error saving to Firebase:", error);
        });
      }
    }
  });

  res.status(200).send('OK'); // Respond to LINE server
});


// Start the server
app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
