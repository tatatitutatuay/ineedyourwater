import { initializeApp } from "https://www.gstatic.com/firebasejs/10.6.0/firebase-app.js";
import { getDatabase, ref, get } from "https://www.gstatic.com/firebasejs/10.6.0/firebase-database.js";

/**
 * Fetch data from Firebase Realtime Database.
 * @returns {Promise<any>} - Resolves to the data from the database or null if no data exists.
 */
export async function getData() {
  try {
    // Firebase configuration
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
    const app = initializeApp(firebaseConfig);
    const database = getDatabase(app);

    // Reference the path and fetch data
    const dbRef = ref(database, 'Sensor');
    const snapshot = await get(dbRef);

    if (snapshot.exists()) {
      return snapshot.val();
    } else {
      console.log("No data available");
      return null;
    }
  } catch (error) {
    console.error("Error fetching data from Firebase:", error);
    throw error;
  }
}
