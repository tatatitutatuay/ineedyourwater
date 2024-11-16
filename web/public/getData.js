//export default function water() {
    const { initializeApp } = require("firebase/app");
    const { getDatabase, ref, set, get } = require("firebase/database");

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
    
    // Reference a location in the database (e.g., `messages/{userId}`)
    const dbRef = ref(database, 'Sensor');
    
    // Save the message to Firebase
    get(dbRef)
        .then((snapshot) => {
            if (snapshot.exists()) {
            console.log("Data retrieved:", snapshot.val());
            } else {
            console.log("No data available at this path.");
            }
        })
        .catch((error) => {
            console.error("Error fetching data:", error);
        });
//}