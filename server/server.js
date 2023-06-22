const express = require('express');
const cors = require("cors");
const mysql = require("mysql");
const app = express();
const PORT = 3001;

const db = mysql.createPool({
    host: "127.0.0.1",
    user: "hanium",
    password: "1234",
    database: "hanium"
});

app.use(cors({
    origin:"*",
    credentials: true,
    optionsSuccessStatus: 200,
}))

app.use(express.urlencoded({extended: true}));

app.listen(PORT, () => {
    console.log(`server running on port${PORT}`);
});

app.get("/api/2023_0417", (req,res) => {
    res.header("Access-Control-Allow-Origin", "*")
    const sqlQuery = "SELECT * FROM 2023_0417";
    db.query(sqlQuery, (err, result) => {
        res.send(result);
    })
})