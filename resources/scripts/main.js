
// Canvas
const canvas = document.getElementById("canvas");

// Game Constants
const ctx = canvas.getContext("2d")
const COLS = 10;
const ROWS = 20;
const BLOCK_SIZE = 30;
canvas.width = COLS * BLOCK_SIZE;
canvas.height = ROWS * BLOCK_SIZE;;
const PIECES = {
  I: {
    blocks: [
      [0, 0, 0, 0],
      [1, 1, 1, 1],
      [0, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    color: "#00F0F0"
  },
  O: {
    blocks: [
      [1, 1],
      [1, 1]
    ],
    color: "#F0F000"
  },
  T: {
    blocks: [
      [0, 1, 0],
      [1, 1, 1],
      [0, 0, 0]
    ],
    color: "#A200F0"
  },
  J: {
    blocks: [
      [1, 0, 0],
      [1, 1, 1],
      [0, 0, 0]
    ],
    color: "#0000F0"
  },
  L: {
    blocks: [
      [0, 0, 1],
      [1, 1, 1],
      [0, 0, 0]
    ],
    color: "#F0A100"
  },
  S: {
    blocks: [
      [0, 1, 1],
      [1, 1, 0],
      [0, 0, 0]
    ],
    color: "#00F000"
  },
  Z: {
    blocks: [
      [1, 1, 0],
      [0, 1, 1],
      [0, 0, 0]
    ],
    color: "#F10000"
  }
};
const MOVE_DELAY = 80;
const MOVE_REPEAT_INTERVAL = 60;
const moveInterval = 1000;
const keyState = {
  KeyA: { pressed: false, lastMoveTime: 0 },
  KeyD: { pressed: false, lastMoveTime: 0 },
  KeyS: { pressed: false, lastMoveTime: 0 },
  KeyR: { pressed: false, lastMoveTime: 0 },
};

// Game Variables
let score = 0;
let lastMoveTime = 0;
let gameOver = false;
let grid = Array.from({ length: ROWS }, () => Array(COLS).fill(0));
let currentPiece;
let isArrowDownPressed = false;
let isArrowLeftPressed = false;
let isArrowRightPressed = false;
let isRotatePressed = false;
let paused = false;
let loggedIn = false;

function drawBlock(x, y, color) {
  const blockPadding = 1;
  const lightColor = getLightColor(color);
  const darkColor = getDarkColor(color);

  ctx.fillStyle = color;
  ctx.fillRect(
    x + blockPadding,
    y + blockPadding,
    BLOCK_SIZE - blockPadding * 2,
    BLOCK_SIZE - blockPadding * 2
  );

  ctx.fillStyle = lightColor;
  ctx.beginPath();
  ctx.moveTo(x + blockPadding, y + blockPadding);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding, y + blockPadding);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding - 4, y + blockPadding + 4);
  ctx.lineTo(x + blockPadding + 4, y + blockPadding + 4);
  ctx.closePath();
  ctx.fill();

  ctx.beginPath();
  ctx.moveTo(x + blockPadding, y + blockPadding);
  ctx.lineTo(x + blockPadding + 4, y + blockPadding + 4);
  ctx.lineTo(x + blockPadding + 4, y + BLOCK_SIZE - blockPadding - 4);
  ctx.lineTo(x + blockPadding, y + BLOCK_SIZE - blockPadding);
  ctx.closePath();
  ctx.fill();

  ctx.fillStyle = darkColor;
  ctx.beginPath();
  ctx.moveTo(x + blockPadding, y + BLOCK_SIZE - blockPadding);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding, y + BLOCK_SIZE - blockPadding);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding - 4, y + BLOCK_SIZE - blockPadding - 4);
  ctx.lineTo(x + blockPadding + 4, y + BLOCK_SIZE - blockPadding - 4);
  ctx.closePath();
  ctx.fill();

  ctx.beginPath();
  ctx.moveTo(x + BLOCK_SIZE - blockPadding, y + blockPadding);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding - 4, y + blockPadding + 4);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding - 4, y + BLOCK_SIZE - blockPadding - 4);
  ctx.lineTo(x + BLOCK_SIZE - blockPadding, y + BLOCK_SIZE - blockPadding);
  ctx.closePath();
  ctx.fill();

  const gradient = ctx.createRadialGradient(
    x + BLOCK_SIZE / 2,
    y + BLOCK_SIZE / 2,
    0,
    x + BLOCK_SIZE / 2,
    y + BLOCK_SIZE / 2,
    BLOCK_SIZE / 2
  );
  gradient.addColorStop(0, 'rgba(255, 255, 255, 0.15)');
  gradient.addColorStop(1, 'rgba(255, 255, 255, 0)');
  ctx.fillStyle = gradient;
  ctx.fillRect(
    x + blockPadding + 4,
    y + blockPadding + 4,
    BLOCK_SIZE - blockPadding * 2 - 8,
    BLOCK_SIZE - blockPadding * 2 - 8
  );
}

function getLightColor(hexColor) {
  const r = parseInt(hexColor.slice(1, 3), 16);
  const g = parseInt(hexColor.slice(3, 5), 16);
  const b = parseInt(hexColor.slice(5, 7), 16);

  const lightenAmount = 40;
  const newR = Math.min(255, r + lightenAmount);
  const newG = Math.min(255, g + lightenAmount);
  const newB = Math.min(255, b + lightenAmount);

  return `rgb(${newR}, ${newG}, ${newB})`;
}

function getDarkColor(hexColor) {
  const r = parseInt(hexColor.slice(1, 3), 16);
  const g = parseInt(hexColor.slice(3, 5), 16);
  const b = parseInt(hexColor.slice(5, 7), 16);

  const darkenAmount = 40;
  const newR = Math.max(0, r - darkenAmount);
  const newG = Math.max(0, g - darkenAmount);
  const newB = Math.max(0, b - darkenAmount);

  return `rgb(${newR}, ${newG}, ${newB})`;
}


class Piece {
  constructor(x, y, type, blocks, color) {
    this.x = x;
    this.y = y;
    this.type = type;
    this.blocks = blocks;
    this.color = color;
    this.rotationState = 0;
  }
  draw() {
    const shadowY = this.getShadowPosition();
    const shadowColor = this.hexToRGBA(this.color, 0.3);
    for (let row = 0; row < this.blocks.length; row++) {
      for (let col = 0; col < this.blocks[row].length; col++) {
        if (this.blocks[row][col]) {
          const x = (this.x + col) * BLOCK_SIZE;
          const y = (shadowY + row) * BLOCK_SIZE;
          ctx.fillStyle = shadowColor;
          ctx.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE);
        }
      }
    }

    for (let row = 0; row < this.blocks.length; row++) {
      for (let col = 0; col < this.blocks[row].length; col++) {
        if (this.blocks[row][col]) {
          const x = (this.x + col) * BLOCK_SIZE;
          const y = (this.y + row) * BLOCK_SIZE;
          drawBlock(x, y, this.color);
        }
      }
    }
  }

  rotate() {
    const previousBlocks = this.blocks.map(row => [...row]);
    const N = this.blocks.length;
    const rotated = Array.from({ length: N }, () => Array(N).fill(0));

    for (let row = 0; row < N; row++) {
      for (let col = 0; col < N; col++) {
        rotated[col][N - 1 - row] = this.blocks[row][col];
      }
    }

    this.blocks = rotated;
    this.rotationState = (this.rotationState + 1) % 4;

    if (!this.isValid()) {
      this.blocks = previousBlocks;
      this.rotationState = (this.rotationState - 1 + 4) % 4;
    }
  }

  isValid(shadowY = null) {
    for (let row = 0; row < this.blocks.length; row++) {
      for (let col = 0; col < this.blocks[row].length; col++) {
        if (this.blocks[row][col]) {
          const newX = this.x + col;
          let newY;
          if (shadowY) {
            newY = shadowY + row;
          } else {
            newY = this.y + row;
          }

          if (newX < 0 || newX >= COLS || newY >= ROWS || newY < 0) {
            return false;
          }

          if (grid[newY][newX]) {
            return false;
          }
        }
      }
    }
    return true;
  }

  getShadowPosition() {
    let shadowY = this.y;

    while (true) {
      shadowY++;
      if (!this.isValid(shadowY)) {
        shadowY--;
        break;
      }
    }

    return shadowY;
  }


  moveLeft() {
    this.x -= 1;
    if (!this.isValid()) {
      this.x += 1;
    }
  }

  moveRight() {
    this.x += 1;
    if (!this.isValid()) {
      this.x -= 1;
    }
  }

  moveDown() {
    this.y += 1;
    if (!this.isValid()) {
      this.y -= 1;
      this.lock();
      return false;
    }
    if (keyState["KeyS"].pressed) {
      score++;
      updateScore();
    }
    return true;
  }

  lock() {
    for (let row = 0; row < this.blocks.length; row++) {
      for (let col = 0; col < this.blocks[row].length; col++) {
        if (this.blocks[row][col]) {
          const gridX = this.x + col;
          const gridY = this.y + row;
          if (gridY >= 0 && gridY < ROWS && gridX >= 0 && gridX < COLS) {
            grid[gridY][gridX] = this.color;
          }
        }
      }
    }

    this.clearRows();

    currentPiece = createPiece();
    if (!currentPiece.isValid()) {
      gameOver = true;
    }
  }

  clearRows() {
    let rowsCleared = 0;
    for (let row = ROWS - 1; row >= 0; row--) {
      if (grid[row].every(cell => cell !== 0)) {
        grid.splice(row, 1);
        grid.unshift(Array(COLS).fill(0));
        rowsCleared++;
        row++;
      }
    }

    if (rowsCleared > 0) {
      const points = [0, 100, 300, 500, 800];
      score += points[rowsCleared] || 0;
      updateScore();
    }
  }

  hexToRGBA(hexColor, alpha) {
    const r = parseInt(hexColor.slice(1, 3), 16);
    const g = parseInt(hexColor.slice(3, 5), 16);
    const b = parseInt(hexColor.slice(5, 7), 16);
    return `rgba(${r}, ${g}, ${b}, ${alpha})`;
  }

}

function updateScore() {
  const scoreElement = document.getElementById("score");
  scoreElement.innerText = score;
  scoreElement.classList.add('score-update');
  setTimeout(() => {
    scoreElement.classList.remove('score-update');
  }, 300);
}

function createPiece() {
  const types = Object.keys(PIECES);
  const type = types[Math.floor(Math.random() * types.length)];
  const { blocks, color } = PIECES[type];
  const x = Math.floor((COLS - blocks[0].length) / 2);
  const y = 0;

  return new Piece(x, y, type, blocks, color);
}

function restartGame() {
  grid = Array.from({ length: ROWS }, () => Array(COLS).fill(0));
  score = 0;
  lastMoveTime = 0;
  gameOver = false;
  paused = false;
  currentPiece = createPiece();
  isArrowDownPressed = false;
  isArrowLeftPressed = false;
  isArrowRightPressed = false;
  isRotatePressed = false;
  updateScore();
}

function addListeners() {
  // Key pressed
  document.addEventListener('keydown', function (event) {
    const key = event.code;
    if (keyState.hasOwnProperty(key)) {
      event.preventDefault();
      if (!keyState[key].pressed) {
        keyState[key].pressed = true;
        keyState[key].lastMoveTime = performance.now() - MOVE_DELAY;
      }
    }
  });

  // Key released
  document.addEventListener('keyup', function (event) {
    const key = event.code;
    if (keyState.hasOwnProperty(key)) {
      keyState[key].pressed = false;
    }
  });

  const restartBtn = document.getElementById("restartBtn");
  restartBtn.addEventListener("click", () => {
    restartGame();
  });

  const pauseBtn = document.getElementById("pauseBtn");
  pauseBtn.addEventListener("click", () => {
    paused = !paused;
    requestAnimationFrame(update);
    if (paused) {
      pauseBtn.innerHTML = "Continue";
    } else {
      pauseBtn.innerHTML = "Pause";
    }
  });
}

function draw() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  ctx.fillStyle = "#181818";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  for (let row = 0; row < ROWS; row++) {
    for (let col = 0; col < COLS; col++) {
      if (grid[row][col]) {
        const x = col * BLOCK_SIZE;
        const y = row * BLOCK_SIZE;
        drawBlock(x, y, grid[row][col]);
      }
    }
  }

  if (currentPiece) {
    currentPiece.draw();
  }

  ctx.strokeStyle = "#404040";
  for (let col = 0; col <= COLS; col++) {
    ctx.beginPath();
    ctx.moveTo(col * BLOCK_SIZE, 0);
    ctx.lineTo(col * BLOCK_SIZE, ROWS * BLOCK_SIZE);
    ctx.stroke();
  }

  for (let row = 0; row <= ROWS; row++) {
    ctx.beginPath();
    ctx.moveTo(0, row * BLOCK_SIZE);
    ctx.lineTo(COLS * BLOCK_SIZE, row * BLOCK_SIZE);
    ctx.stroke();
  }

  if (gameOver) {
    ctx.fillStyle = "white";
    ctx.font = "40px Arial";
    ctx.textAlign = "center";
    ctx.fillText("Game Over", canvas.width / 2, canvas.height / 2);
  }
}

function sendScore() {
  fetch("http://localhost:8080/score", {
    method: "POST",
    body: JSON.stringify({
      score: score,
    }),
    headers: {
      "Content-type": "application/json; charset=UTF-8"
    }
  });
}

function update(timestamp) {
  if (!paused) {

    if (gameOver) {
      draw();
      sendScore();
      return;
    }

    if (!lastMoveTime) {
      lastMoveTime = timestamp;
    }

    const delta = timestamp - lastMoveTime;

    if (delta > moveInterval) {
      currentPiece.moveDown();
      lastMoveTime = timestamp;
    }

    for (const key in keyState) {
      if (keyState[key].pressed) {
        const currentTime = timestamp;
        const timeSinceLastMove = currentTime - keyState[key].lastMoveTime;

        if (timeSinceLastMove > MOVE_DELAY) {
          switch (key) {
            case 'KeyA':
              currentPiece.moveLeft();
              break;
            case 'KeyD':
              currentPiece.moveRight();
              break;
            case 'KeyS':
              currentPiece.moveDown();
              break;
            case 'KeyR':
              currentPiece.rotate();
              break;
            default:
              break;
          }

          keyState[key].lastMoveTime = currentTime;
          keyState[key].lastMoveTime += MOVE_REPEAT_INTERVAL;
        }
      }
    }

    draw();
    requestAnimationFrame(update);
  } else {
    ctx.fillStyle = "white";
    ctx.font = "40px Arial";
    ctx.textAlign = "center";
    ctx.fillText("Game Paused", canvas.width / 2, canvas.height / 2)
  }
}

function register() {
  const registerForm = document.getElementById("registerForm");
  registerForm.addEventListener("submit", (event) => {
    event.preventDefault();
    const username = document.getElementById("registerUsername").value;
    const password = document.getElementById("registerPassword").value;
    const confirmPassword = document.getElementById("confirmPassword").value;

    if (password !== confirmPassword) {
      alert("Passwords do not match");
      return;
    }

    fetch("http://localhost:8080/register", {
      method: "POST",
      body: JSON.stringify({
        username: username,
        password: password
      }),
      headers: {
        "Content-Type": "application/json"
      }
    }).then(response => {
      if (response.ok) {
        // document.getElementById("register").style.display = "none";
        // document.getElementById("login").style.display = "block";
        alert("User registered successfully");
      } else {
        alert("Username already exists");
      }
    });
  });
}

function login() {
  const loginForm = document.getElementById("loginForm");
  loginForm.addEventListener("submit", (event) => {
    event.preventDefault();
    const username = document.getElementById("loginUsername").value;
    const password = document.getElementById("loginPassword").value;

    fetch("http://localhost:8080/login", {
      method: "POST",
      body: JSON.stringify({
        username: username,
        password: password
      }),
      headers: {
        "Content-Type": "application/json"
      }
    }).then(response => {
      if (response.ok) {
        document.getElementById("authContainer").style.display = "none";
        document.getElementById("gameContainer").classList.remove("hidden");
        loggedIn = true; loggedIn = true;
        startGame();
      } else if (response.status == 404) {
        alert(`User "${username}" not found or password is incorrect`);
      }
    });
  });
}

function addAuthListeners() {
  login();
  register();
}

function startGame() {
  if (loggedIn) {
    currentPiece = createPiece();
    draw();
    updateScore();
    addListeners();
    requestAnimationFrame(update);
  }
}

window.addEventListener("load", () => {
  addAuthListeners();
  startGame();
});

document.addEventListener("DOMContentLoaded", () => {
  const loginTab = document.getElementById("loginTab");
  const registerTab = document.getElementById("registerTab");
  const loginForm = document.getElementById("loginForm");
  const registerForm = document.getElementById("registerForm");

  loginTab.addEventListener("click", () => {
    loginTab.classList.add("active");
    registerTab.classList.remove("active");
    loginForm.classList.remove("hidden");
    registerForm.classList.add("hidden");
  });

  registerTab.addEventListener("click", () => {
    registerTab.classList.add("active");
    loginTab.classList.remove("active");
    registerForm.classList.remove("hidden");
    loginForm.classList.add("hidden");
  });
});
