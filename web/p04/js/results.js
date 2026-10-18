// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 4: Adders and comparators  |  fetch adder_results.json and fill the results table
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// JSON is just text that looks like Python dicts and lists. fetch() downloads it,
// response.json() turns it into JavaScript objects and arrays, and we loop over them.

// Add one <tr> to the table for one test result object, e.g.
// {"bench": "B01", "chip": "74HC283", "tested": 512, "failed": 0}
function addRow(tbody, result) {
  const row = document.createElement("tr");

  // A small helper: make a <td>, put text in it, add it to the row.
  // We always use textContent (never innerHTML) so the data is shown as text
  // and can never be run as HTML or JavaScript.
  function cell(text, className) {
    const td = document.createElement("td");
    td.textContent = text;
    if (className) {
      td.className = className;
    }
    row.appendChild(td);
  }

  cell(result.bench);
  cell(result.chip);
  cell(String(result.tested));
  cell(String(result.failed));
  if (result.failed === 0) {
    cell("PASS", "ok");
  } else {
    cell("FAIL", "error");
  }
  cell(result.note || "");   // "" when the object has no note

  tbody.appendChild(row);
}

// async lets us use "await": wait for the download without freezing the page.
async function loadResults() {
  const status = document.getElementById("status");

  // fetch() cannot read files when the page was opened with file:// (see the note on the page).
  if (window.location.protocol === "file:") {
    status.textContent = "This page was opened as a file. Start tools/serve.py and open it via http://127.0.0.1:8000/ instead.";
    status.className = "error";
    return;
  }

  try {
    const response = await fetch("adder_results.json");
    if (!response.ok) {
      throw new Error("HTTP " + response.status);
    }
    const results = await response.json();   // an array of objects

    const tbody = document.getElementById("results-body");
    let passed = 0;
    for (const result of results) {
      addRow(tbody, result);
      if (result.failed === 0) {
        passed = passed + 1;
      }
    }

    document.getElementById("results-table").hidden = false;
    status.textContent = passed + " of " + results.length + " test runs passed.";
  } catch (error) {
    status.textContent = "Could not load adder_results.json (" + error.message + ").";
    status.className = "error";
  }
}

loadResults();
