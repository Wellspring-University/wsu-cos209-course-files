-- WSU-COS 209 The Circuit-to-Cloud Workshop
-- Practical 12: Cloud logger  |  database schema shared by the Flask and the Node server
-- (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
--
-- One table, one row per reading sent by an ESP32. Both servers run this file at start-up;
-- "IF NOT EXISTS" makes it safe to run again and again.

CREATE TABLE IF NOT EXISTS readings (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  device TEXT NOT NULL,
  light INTEGER NOT NULL,
  led INTEGER NOT NULL,          -- 0 or 1
  created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
