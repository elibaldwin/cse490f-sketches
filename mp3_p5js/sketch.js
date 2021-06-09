

let missiles, enemies, walls;
let player;

const M_SIZE = 6; // missile size
const M_LEN = 12;   // missile trail length
const M_TURN = 4;  // missile angular turn (maximum) speed
const M_SPEED = 4;
const P_SPEED = 5;

// filter incoming serial distance info
let distFilter;
let minDist = 15;
let maxDist = 80;
let playerDist = (minDist + maxDist) / 2;

let timescale = 1.0;
let minTimescale = 0.5;
let maxTimescale = 1.5;
let lastFrameMS = 0;
let frametimeRef = 1000/60;

let handpose;
let video;
let predictions = [];
let poseOptions = { maxContinuousChecks: 1 };


// filter hand position
let xFilter, yFilter;
let xPos = 0;
let yPos = 0;

let pHtmlMsg;
let serialOptions = { baudRate: 115200  };
let serial;

let levelNum = 1;
let levelStep = 0;
let waveTimestamp = 0;

// toggles between using webcam hand position or mouse for player control
const USE_POSE = false;
const USE_SERIAL = false;

let heading, scoreboard, comboboard;
let score = 0;
let combo = 1;

let init = USE_POSE;
let gameover = false;

function setup() {
  heading = createElement('h1', 'Title');
  heading.position(10, 0);

  scoreboard = createElement('h2', 'Score: 0');
  scoreboard.position(600, 10);

  comboboard = createElement('h3', 'Combo: 1x');
  comboboard.position(900, 15);

  pHtmlMsg = createP();
  
  pHtmlMsg.position(1280-60, 25);

  let canvas = createCanvas(1280, 720);
  canvas.position(0, 60);

  video = createCapture(VIDEO);
  video.size(384, 216);
  video.position(0, height + 65);

  if (USE_POSE) {
    handpose = ml5.handpose(video, modelReady);
    handpose.on("predict", results => {
      predictions = results;
    });
  } else if (USE_SERIAL) {
    setupSerial();
    distFilter = new OneEuroFilter(60);
  }

  walls = new Group();
  missiles = new Group();
  enemies = new Group();

  walls.add(createSprite(5, height/2, 5, height - 10))
  walls.add(createSprite(width-5, height/2, 5, height-10))
  walls.add(createSprite(width/2, 5, width-10, 5))
  walls.add(createSprite(width/2, height-5, width-10, 5))

  player = createSprite(width/2, height/2, 30, 30);
  player.setCollider('circle', 0, 0, 15);
  player.draw = function () {
    fill(this.shapeColor);
    circle(0, 0, 30);
  }
  player.shapeColor = color(50, 50, 220);

  reset();

  angleMode(DEGREES);
  //frameRate(5);

  xFilter = new OneEuroFilter(60, 1.0, 0.002, 1.0);
  yFilter = new OneEuroFilter(60, 1.0, 0.002, 1.0);

}

function reset() {
  player.position.x = width/2;
  player.position.y = height/2;
  player.hitpoints = 50;

  missiles.removeSprites();
  enemies.removeSprites();

  levelNum = 1;
  levelStep = 0;
  waveTimestamp = millis() + 10000;

  score = 0;
  combo = 1;
  gameover = false;
}

function draw() {

  background(255,255,255);

  if (!init && !gameover) {
    updateLevel();

    if (USE_POSE) {
      updatePose();
      updateTimescale();
      updatePlayer(xPos, yPos);
    } else {
      updateTimescale();
      updatePlayer(mouseX, mouseY);
    }
    
    updateMissiles();
    updateEnemies();

    drawSprites();
    pHtmlMsg.html(player.hitpoints);
  } else if (gameover) {
    heading.html("Game Over");
    scoreboard.html("Score: " + int(score));
  }
  
}

function updateTimescale() {
  timescale = map(playerDist, minDist, maxDist, minTimescale, maxTimescale);
  let frametime = millis() - lastFrameMS;
  timescale = timescale * (frametime / frametimeRef);

  lastFrameMS = millis();
}

function updateLevel() {
  scoreboard.html("Score: " + int(score));
  comboboard.html("Combo: " + combo + "x");
  if (levelNum > 3) {
    return;
  }

  let ms = millis();
  if (ms > waveTimestamp || enemies.size() === 0) {
    if (levelStep == 0 && enemies.size() != 0) {
      return;
    } 

    heading.html("Level " + levelNum + ": " + LEVELS[levelNum].title);

    let wave = LEVELS[levelNum].level[levelStep];
    for (let i = 0; i < wave.enemies.length; i++) {
      spawnEnemy(wave.enemies[i]);
    }

    waveTimestamp = ms + wave.duration * 1000;

    levelStep += 1;
    if (levelStep >= LEVELS[levelNum].level.length) {
      levelStep = 0;
      levelNum += 1;
    }
  }
}

function updatePose() {
  for (let i = 0; i < predictions.length; i++) {
    const pred = predictions[i];
    let xSum = 0;
    let ySum = 0;
    for (let j = 0; j < pred.landmarks.length; j++) {
      xSum += pred.landmarks[j][0];
      ySum += pred.landmarks[j][1];
    }
    let n = pred.landmarks.length;
    let timestep = millis() / 1000.0;

    xPos = map(xFilter.filter(xSum/n, timestep), 0, 640, 1380, -100);
    yPos = map(yFilter.filter(ySum/n, timestep), 0, 360, -80, 800);
  }
}

function mouseClicked() {
  spawnEnemy(int(random(1, 4)));
}

function keyPressed() {
  if (key === 's' && USE_SERIAL) {
    if (!serial.isOpen()) {
       serial.connectAndOpen(null, serialOptions);
    }
  } else if (key === 'r') {
    reset();
  }
}

function updatePlayer(xref, yref) {
  if (player.hitpoints <= 0) {
    gameover=true;
    return;
  }

  if (player.position.x < 0 || player.position.y < 0) {
    player.position.x = width / 2;
    player.position.y = height / 2;
  }
  stroke(150);
  line(xref-10, yref, xref+10, yref);
  line(xref, yref-10, xref, yref+10);

  let dx = xref-player.position.x;
  let dy = yref-player.position.y;

  let dir = createVector(dx, dy);

  player.setSpeed(constrain(dir.mag(), 0, P_SPEED)*timescale, dir.heading());
  player.collide(walls);
}

function updateMissiles() {

  for (let i = 0; i < missiles.size(); i++) {
    let m = missiles.get(i);

    // missiles eventually explode when they get old enough
    m.lifetime -= timescale;
    if (m.lifetime < 5) {
      goBoom(m.position.x, m.position.y);
      m.remove();
      continue;
    }

    if (frameCount % m.trailstride == 0) {
      m.posHist.unshift(m.position.x, m.position.y);
      if (m.posHist.length > M_LEN*2) {
        m.posHist.pop();
        m.posHist.pop();
      }
    }

    m.maxSpeed = m.speed * timescale;
    m.attractionPoint((m.maxSpeed/20)*timescale, player.position.x, player.position.y);
  }

  missiles.overlap(walls, missileHit);
  missiles.overlap(player, missileHit);
  missiles.overlap(enemies, missileHit);
}

function updateEnemies() {
  for (let i = 0; i < enemies.size(); i++) {
    let e = enemies.get(i);
    e.debug = mouseIsPressed;
    e.aiUpdate();
  }
}

function fireMissile(x, y, speed, mColor, direction, strength=1, lifespan=500, trailstride=1) {
  let m = createSprite(x, y, M_SIZE, M_SIZE);
  m.setCollider("circle", 0, 0, M_SIZE/2);

  m.color = mColor;
  m.setSpeed(speed, direction);
  m.posHist = [x, y];
  m.maxSpeed = speed;
  m.speed = speed;
  m.friction = 0.03;
  m.strength = strength;
  m.lifetime = lifespan;
  m.trailstride = trailstride;

  m.draw = function() {

    let px = this.position.x;
    let py = this.position.y;

    this.color.setAlpha(210);
    fill(this.color);
    circle(0, 0, M_SIZE);

    for (let i = 0; i < this.posHist.length; i+=2) {
      this.color.setAlpha(random(150, 210)-5*i);
      fill(this.color);
      
      let drawx = this.posHist[i]-px;
      let drawy = this.posHist[i+1]-py;

      circle(drawx, drawy, M_SIZE - i/6);
    }
  }

  missiles.add(m);
}

function missileHit(missile, object) {
  if ('hitpoints' in object) {
    object.hitpoints -= missile.strength;
    if(object.hitpoints <= 0) {
      if ('scoreValue' in object) {
        score += object.scoreValue * combo * map(playerDist, minDist, maxDist, 0.25, 1.75);
        combo = min(combo+1, 8);
      }
      if (object != player) {
        object.remove();
        goBoom(object.position.x, object.position.y, 100);
      }
    }
  }

  if (object == player) {
    combo = 1;
  }

  goBoom(missile.position.x, missile.position.y);
  missile.remove();
}

/*
Enemy Types:
1: basic:
  - choose a random screen location
  - move to that spot
  - once reached, rotate in place and fire towards player at consistent interval
   (missile type 1)

2: moving:
  - move slowly between randomly chosen target points on screen
  - fire missiles towards player at consistent interval
   (missile type 1)

3: kamikaze:
  - choose a random screen location
  - move to that spot
  - after a set interval, explode and shoot out a burst of missiles in all directions
   (missile type 2)


*/

function spawnEnemy(enemyType) {
  // start out with sprite off-screen
  let startX, startY;
  let side = int(random(4));
  if (side == 0) {
    startX = random(0, width);
    startY = -100;
  } else if (side == 1) {
    startX = width + 100;
    startY = random(0, height);
  } else if (side == 2) {
    startX = random(0, width);
    startY = height + 100;
  } else {
    startX = -100;
    startY = random(0, height);
  }

  let newEnemy = createSprite(startX, startY);

  if (enemyType == 1) {

    // initialize enemy state
    newEnemy.setCollider("circle", 0, 0, 20);
    newEnemy.targetPos = createVector(random(150, width-150), random(100, height-100));
    newEnemy.reachedTarget = false;
    newEnemy.speed = 3;
    newEnemy.rotateToDirection = true;
    newEnemy.fireInterval = 4000;
    newEnemy.hitpoints = 2;
    newEnemy.scoreValue = 100;

    newEnemy.draw = function () {
      fill(50);
      stroke(255, 215, 60);
      strokeWeight(5);
      circle(0, 0, 40);
      stroke(50);
      line(0, 0, 30, 0);
    }

    newEnemy.aiUpdate = function () {
      if (!this.reachedTarget) {

        let diff = p5.Vector.sub(this.targetPos, this.position);
        if (diff.mag() < 1) {
          this.reachedTarget = true;
          this.fireTimestamp = millis() + this.fireInterval;
        }
        this.setSpeed(constrain(diff.mag(), 0, this.speed)*timescale, diff.heading());

      } else {

        let pDir = p5.Vector.sub(player.position, this.position);
        pDir.normalize();
        pDir.mult(35);
        this.rotation = pDir.heading();
        this.setSpeed(0);
        if (millis() > this.fireTimestamp) {
          fireMissile(this.position.x + pDir.x, this.position.y + pDir.y, 
            M_SPEED, color(255, 0, 0), pDir.heading(), 1, 500, 1);
          this.fireTimestamp += this.fireInterval;
        }

      }
    }
    
  } else if (enemyType == 2) {

    // initialize enemy state
    newEnemy.setCollider("circle", 0, 0, 25);
    newEnemy.targetPos = createVector(random(150, width-150), random(100, height-100));
    newEnemy.maxSpeed = 1.5;
    newEnemy.rotateToDirection = false;
    newEnemy.fireInterval = 3500;
    newEnemy.hitpoints = 4;
    newEnemy.fireTimestamp = millis() + newEnemy.fireInterval;
    newEnemy.scoreValue = 200;

    newEnemy.draw = function () {
      fill(50);
      circle(0, 0, 50);
      fill(90, 160, 220);
      circle(0, 0, 30);
      stroke(70, 130, 180);
      strokeWeight(5);
      line(0, 0, 30, 0);
    }

    newEnemy.aiUpdate = function () {
      let diff = p5.Vector.sub(this.targetPos, this.position);
      if (diff.mag() < 50) {
        this.targetPos = createVector(random(150, width-150), random(100, height-100));
      }
      this.attractionPoint((this.maxSpeed/15)*timescale, this.targetPos.x, this.targetPos.y);

      let pDir = p5.Vector.sub(player.position, this.position);
      pDir.normalize();
      pDir.mult(35);
      this.rotation = pDir.heading();

      if (millis() > this.fireTimestamp) {
        fireMissile(this.position.x + pDir.x, this.position.y + pDir.y, 
          M_SPEED, color(255, 0, 0), pDir.heading(), 1, 500, 1);
        this.fireTimestamp += this.fireInterval;
      }
    }

  } else if (enemyType == 3) {

    // initialize enemy state
    newEnemy.setCollider("circle", 0, 0, 20);
    newEnemy.targetPos = createVector(random(150, width-150), random(100, height-100));
    newEnemy.reachedTarget = false;
    newEnemy.speed = 4;
    newEnemy.rotateToDirection = false;
    newEnemy.hitpoints = 3;
    newEnemy.fireInterval = 7000;
    newEnemy.scoreValue = 300;

    newEnemy.draw = function () {
      fill(0, 0);
      stroke(85, 200, 50);
      strokeWeight(5);
      circle(0, 0, 40);
      fill(255, 0, 0, abs(sin(millis()/3))*255);
      noStroke();
      circle(0, 0, 5);
    }

    newEnemy.aiUpdate = function () {
      if (!this.reachedTarget) {

        let diff = p5.Vector.sub(this.targetPos, this.position);
        if (diff.mag() < 1) {
          this.reachedTarget = true;
          this.fireTimestamp = millis() + this.fireInterval;
        }
        this.setSpeed(constrain(diff.mag(), 0, this.speed)*timescale, diff.heading());

      } else {
        this.setSpeed(0);
        if (millis() > this.fireTimestamp) {
          for (let theta = 0; theta < 360; theta += 45) {
            let angle = theta + random(-15, 15);
            fireMissile(this.position.x + cos(angle)*25, this.position.y + sin(angle)*25, 
              M_SPEED*1.5, color(85, 200, 50), angle, 2, random(450, 550), 3);
          }
          this.remove()
          goBoom(this.position.x, this.position.y, 100);
        }

      }
    }

  }

  enemies.add(newEnemy);
}

function goBoom(x, y, size=60) {
  // TODO: better texture for explosion sprite?
  let boom = createSprite(x, y);
  boom.life = 15;
  boom.draw = function() {
    fill(255, 145, 50, map(this.life, 0, 15, 0, 150));
    circle(0, 0, size);
  }
}

function setupSerial() {
  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  // If we have previously approved ports, attempt to connect with them
  serial.autoConnectAndOpenPreviouslyApprovedPort(serialOptions);

  // Add in a lil <p> element to provide messages. This is optional
  pHtmlMsg.html("Click anywhere on this page to open the serial connection dialog");
}

/**
 * Callback function by serial.js when there is an error on web serial
 * 
 * @param {} eventSender 
 */
 function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
  pHtmlMsg.html(error);
}

/**
 * Callback function by serial.js when web serial connection is opened
 * 
 * @param {} eventSender 
 */
function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");
  pHtmlMsg.html("Serial connection opened successfully");
}

/**
 * Callback function by serial.js when web serial connection is closed
 * 
 * @param {} eventSender 
 */
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
  pHtmlMsg.html("onSerialConnectionClosed");
}

/**
 * Callback function serial.js when new web serial data is received
 * 
 * @param {*} eventSender 
 * @param {String} newData new data received over serial
 */
function onSerialDataReceived(eventSender, newData) {
  let newDist = int(newData);
  console.log(newDist);
  if (newDist <= 100) {
    playerDist = distFilter.filter(newDist, millis()/1000.0);
  }
  serial.writeLine(player.hitpoints);
}


function modelReady() {
  console.log("handpose model is ready.");
  if (USE_SERIAL) {
    setupSerial();
    distFilter = new OneEuroFilter(60);
  }
  player.position.x = width / 2;
  player.position.y = height / 2;
  waveTimestamp = millis() + 5000;
  lastFrameMS = millis();
  init = false;
}


const LEVELS = {
  1 : {
    title : "Getting Started",
    level : [
      { duration: 10, enemies: [1, 1] },
      { duration: 10, enemies: [1, 1, 1]},
      { duration: 10, enemies: [1, 1, 1, 1]}
    ]
  },

  2 : {
    title : "Moving Target",
    level : [
      { duration: 5,  enemies: [1, 2] },
      { duration: 2,  enemies: [1, 2] },
      { duration: 5,  enemies: [1, 2] },
      { duration: 10, enemies: [1, 1, 2, 2] }
    ]
  }, 

  3: {
    title: "Boom",
    level : [
      { duration: 10, enemies: [3] },
      { duration: 10, enemies: [1, 2, 2, 3] },
      { duration: 10, enemies: [1, 1, 2, 2, 3, 3] },
      { duration: 3,  enemies: [2, 2, 2, 2] },
      { duration: 2,  enemies: [3] },
      { duration: 2,  enemies: [3] },
      { duration: 2,  enemies: [1, 1, 2, 2] },
      { duration: 2,  enemies: [3, 3] }
    ]
  }
};