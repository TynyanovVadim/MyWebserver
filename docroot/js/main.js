// This file serves as the entry point for the JavaScript code. It initializes the canvas, sets up event listeners, and manages the application logic for the painting functionality.

const canvas = document.getElementById('paintCanvas');
const ctx = canvas.getContext('2d');

let painting = false;

function resizeCanvas() {
    canvas.width = window.innerWidth * 0.8;
    canvas.height = window.innerHeight * 0.6;
    ctx.clearRect(0, 0, canvas.width, canvas.height); // Clear canvas on resize
}

const clearButton = document.getElementById('clearCanvas');
clearButton.addEventListener('click', () => {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
});

canvas.addEventListener('mousedown', startPosition);
canvas.addEventListener('mouseup', endPosition);
canvas.addEventListener('mousemove', draw);

window.addEventListener('resize', resizeCanvas);
resizeCanvas(); // Set initial size