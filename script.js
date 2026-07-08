/* ======================================================
   ARK SCRIPT - FUNCIONALIDAD TOTAL
====================================================== */

// 1. TEMA CLARO/OSCURO
const themeToggle = document.getElementById('theme-toggle');
const html = document.documentElement;
const savedTheme = localStorage.getItem('theme') || 'dark';
html.setAttribute('data-theme', savedTheme);

themeToggle.addEventListener('click', () => {
    const currentTheme = html.getAttribute('data-theme');
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    html.setAttribute('data-theme', newTheme);
    localStorage.setItem('theme', newTheme);
});

// 2. NAVBAR SCROLL Y HAMBURGUESA
const navbar = document.querySelector('.navbar');
const hamburger = document.getElementById('hamburger');
const navLinks = document.querySelector('.nav-links');

window.addEventListener('scroll', () => {
    if (window.scrollY > 40) navbar.classList.add('scrolled');
    else navbar.classList.remove('scrolled');
});

hamburger.addEventListener('click', () => {
    navLinks.classList.toggle('active');
});

// 3. CURSOR GLOW EFFECT
const cursorGlow = document.getElementById('cursor-glow');
document.addEventListener('mousemove', (e) => {
    cursorGlow.style.left = e.clientX + 'px';
    cursorGlow.style.top = e.clientY + 'px';
});

// 4. SISTEMA DE PARTÍCULAS DE FONDO
const canvas = document.getElementById('particles-canvas');
const ctx = canvas.getContext('2d');
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

let particlesArray = [];
class Particle {
    constructor() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.size = Math.random() * 2;
        this.speedX = (Math.random() * 1) - 0.5;
        this.speedY = (Math.random() * 1) - 0.5;
    }
    update() {
        this.x += this.speedX;
        this.y += this.speedY;
        if (this.x > canvas.width || this.x < 0) this.speedX *= -1;
        if (this.y > canvas.height || this.y < 0) this.speedY *= -1;
    }
    draw() {
        ctx.fillStyle = html.getAttribute('data-theme') === 'dark' ? 'rgba(255,255,255,0.5)' : 'rgba(0,0,0,0.3)';
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
    }
}
function initParticles() {
    particlesArray = [];
    for (let i = 0; i < 60; i++) particlesArray.push(new Particle());
}
function animateParticles() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    for (let i = 0; i < particlesArray.length; i++) {
        particlesArray[i].update();
        particlesArray[i].draw();
    }
    requestAnimationFrame(animateParticles);
}
initParticles();
animateParticles();
window.addEventListener('resize', () => {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    initParticles();
});

// 5. GITHUB STATS
async function fetchGitHubStats() {
    try {
        const response = await fetch('https://api.github.com/repos/Dev-DooM8002/Ark');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('gh-stars').innerText = data.stargazers_count;
            document.getElementById('gh-forks').innerText = data.forks_count;
        }
    } catch (error) {
        console.error("No se pudieron cargar las stats de GitHub.");
    }
}
fetchGitHubStats();

// 6. FAKE PLAYGROUND - SYNTAX HIGHLIGHTING (Real Time)
const codeEditor = document.getElementById('code-editor');
const codeHighlight = document.getElementById('code-highlight');

function applyHighlight(text) {
    // 1. Escapar HTML
    let highlighted = text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
    
    // 2. Secciones (e.g. .data:, .start:, .end)
    highlighted = highlighted.replace(/(\.[a-zA-Z]+:?)/g, '<span class="sy-sec">$1</span>');
    
    // 3. Strings ("texto")
    highlighted = highlighted.replace(/(&quot;.*?&quot;)/g, '<span class="sy-str">$1</span>');
    
    // 4. Keywords (var, fn, int, str, pnl, etc)
    const keywords = ["var", "fn", "int", "str", "bool", "if", "elif", "else", "loop", "uloop", "break", "jump", "and", "or", "not", "pnl", "pil", "cin"];
    const kwRegex = new RegExp(`\\b(${keywords.join('|')})\\b`, 'g');
    highlighted = highlighted.replace(kwRegex, '<span class="sy-kw">$1</span>');

    // 5. Comentarios (// comentario)
    highlighted = highlighted.replace(/(\/\/.*)/g, '<span class="sy-com">$1</span>');

    return highlighted;
}

function syncEditor() {
    codeHighlight.innerHTML = applyHighlight(codeEditor.value);
}
codeEditor.addEventListener('input', syncEditor);
codeEditor.addEventListener('scroll', () => {
    codeHighlight.parentElement.scrollTop = codeEditor.scrollTop;
    codeHighlight.parentElement.scrollLeft = codeEditor.scrollLeft;
});
// Trigger inicial
syncEditor();

// 7. FAKE PLAYGROUND - SIMULACIÓN DE TERMINAL
const runBtn = document.getElementById('run-btn');
const terminalOutput = document.getElementById('terminal-output');

runBtn.addEventListener('click', () => {
    if(runBtn.disabled) return;
    runBtn.disabled = true;
    
    const userCode = codeEditor.value;
    // Intentar extraer strings dentro de pnl() de forma súper sencilla para darle realismo
    let customOutput = "Hello World";
    const pnlMatch = userCode.match(/pnl\(\s*"([^"]+)"/);
    if(pnlMatch && pnlMatch[1]) {
        customOutput = pnlMatch[1];
    } else if (userCode.includes("<<")) {
        customOutput = "Construyendo Ark..."; // Fallback simple para la concatenación por defecto
    }

    const steps = [
        "Lexing source............. OK\n",
        "Parsing AST............... OK\n",
        "Semantic analysis......... OK\n",
        "Generating Assembly....... OK\n",
        "Running NASM.............. OK\n",
        "Linking executable........ OK\n",
        "--------------------------------\n",
        "Executable: ./main\n\n",
        `<span style="color:#28c840">${customOutput}</span>\n`
    ];

    terminalOutput.innerHTML = "$ ark main.ark\n\n";
    let stepIndex = 0;

    function typeTerminal() {
        if (stepIndex < steps.length) {
            terminalOutput.innerHTML += steps[stepIndex];
            stepIndex++;
            terminalOutput.innerHTML += '<span id="term-cursor">█</span>';
            // Scroll to bottom
            terminalOutput.scrollTop = terminalOutput.scrollHeight;
            
            // Tiempo variable para simular trabajo
            setTimeout(() => {
                terminalOutput.innerHTML = terminalOutput.innerHTML.replace('<span id="term-cursor">█</span>', '');
                typeTerminal();
            }, stepIndex === 4 || stepIndex === 5 ? 400 : 150); // Simular que NASM y Linker tardan más
        } else {
            terminalOutput.innerHTML += '<br>$ <span id="term-cursor">█</span>';
            runBtn.disabled = false;
        }
    }

    typeTerminal();
});