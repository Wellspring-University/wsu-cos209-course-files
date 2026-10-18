// WSU-COS 209 The Circuit-to-Cloud Workshop
// Practical 1: Number systems  |  live decimal converter for pages/p01.html (STARTER)
// (c) 2026 Wellspring University, Faculty of Computing. For registered WSU-COS 209 students.
//
// Every time the user types, we read the decimal number, check it is 0..255,
// and show the same byte as binary, hex, BCD, Gray code and a signed value.
// Each conversion is a small function that takes a number and returns a string,
// so you can test them one at a time in the browser console, e.g. toGray(45).
//
// STARTER: toBinary and toHex are done. Finish toBCD, toGray and toSigned (see the TODO
// comments). Until you do, those three boxes show "TODO". Check your answers against the
// p01_dip_reader sketch: 45 -> gray 00111011, 200 -> signed -56, 255 -> BCD 0010 0101 0101.

// Pad a string on the left with zeros until it is `width` characters long.
// Example: padBits("101", 8) gives "00000101".
function padBits(text, width) {
  return text.padStart(width, "0");
}

// 8-bit binary, e.g. 45 -> "00101101"
function toBinary(n) {
  return padBits(n.toString(2), 8);
}

// Hexadecimal with the 0x prefix, two digits, upper case, e.g. 45 -> "0x2D"
function toHex(n) {
  return "0x" + n.toString(16).toUpperCase().padStart(2, "0");
}

// BCD: each DECIMAL digit becomes its own 4-bit group.
// 45 -> digits "4","5" -> "0100 0101".  255 -> "0010 0101 0101".
function toBCD(n) {
  // TODO 1: split String(n) into its digits, turn each digit into a 4-bit binary string
  // (use padBits(Number(d).toString(2), 4)), and join the groups with a space.
  // Hint: it is a loop over String(n).split("").
  return "TODO";
}

// Gray code: n XOR (n shifted right by one). Neighbouring numbers
// differ in exactly one bit, which is why rotary encoders use it.
function toGray(n) {
  // TODO 2: compute n ^ (n >> 1) and return it as an 8-bit binary string (use padBits).
  return "TODO";
}

// Two's complement: read the same 8 bits as a SIGNED number.
// Bit 7 is worth -128 instead of +128, so 200 (11001000) reads as -56.
function toSigned(n) {
  // TODO 3: if n is 128 or more, return String(n - 256); otherwise return String(n).
  return "TODO";
}

// Show the same text in every output box (used to clear them).
function showAll(text) {
  for (const id of ["out-bin", "out-hex", "out-bcd", "out-gray", "out-signed"]) {
    document.getElementById(id).textContent = text;
  }
}

// Called on every keystroke in the input box.
function update() {
  const input = document.getElementById("decimal");
  const error = document.getElementById("decimal-error");
  const raw = input.value.trim();

  if (raw === "") {               // empty box: nothing to show yet
    error.textContent = "";
    input.removeAttribute("aria-invalid");
    showAll("-");
    return;
  }

  const n = Number(raw);
  if (!Number.isInteger(n) || n < 0 || n > 255) {
    error.textContent = "Please type a whole number from 0 to 255 (one byte).";
    input.setAttribute("aria-invalid", "true");
    showAll("-");
    return;
  }

  error.textContent = "";
  input.removeAttribute("aria-invalid");
  document.getElementById("out-bin").textContent = toBinary(n);
  document.getElementById("out-hex").textContent = toHex(n);
  document.getElementById("out-bcd").textContent = toBCD(n);
  document.getElementById("out-gray").textContent = toGray(n);
  document.getElementById("out-signed").textContent = toSigned(n);
}

// Wire up the page once the script loads (the <script> tag is at the end of <body>).
document.getElementById("decimal").addEventListener("input", update);
// Pressing Enter should not reload the page.
document.getElementById("converter").addEventListener("submit", function (event) {
  event.preventDefault();
});
update();
