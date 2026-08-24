// Optical Simulation and Pre-compensation Engine
class OpticalEngine {
    constructor() {
        this.canvas = document.getElementById('main-canvas');
        this.ctx = this.canvas.getContext('2d');
        
        // Offscreen canvases for multi-pass rendering
        this.sourceCanvas = document.createElement('canvas');
        this.sourceCtx = this.sourceCanvas.getContext('2d');
        
        this.processedCanvas = document.createElement('canvas');
        this.processedCtx = this.processedCanvas.getContext('2d');

        // State parameters
        this.params = {
            sphere: -2.00,
            cylinder: -1.50,
            axis: 90,
            distance: 60,
            skewX: 0.0,
            skewY: 0.0,
            anamorphic: 1.08,
            deconv: 1.4,
            contrast: 1.25,
            sourceType: 'text', // 'text', 'chart', 'image'
            viewMode: 'precomp', // 'precomp', 'split', 'original'
            splitPos: 0.5,
            fontSize: 22,
            customText: 'La óptica computacional permite pre-compensar la distorsión del astigmatismo y refracción. 1234567890 AaBbCcDdEe',
            currentImagePreset: 'code',
            customImage: null
        };

        this.width = 1000;
        this.height = 650;

        this.initCanvases();
        this.setupEventListeners();
        this.renderSource();
        this.processAndDraw();
    }

    initCanvases() {
        [this.canvas, this.sourceCanvas, this.processedCanvas].forEach(c => {
            c.width = this.width;
            c.height = this.height;
        });
    }

    setupEventListeners() {
        // Range Inputs
        const bindSlider = (id, paramKey, formatFn) => {
            const input = document.getElementById(id);
            const display = document.getElementById(`val-${id}`);
            input.addEventListener('input', (e) => {
                const val = parseFloat(e.target.value);
                this.params[paramKey] = val;
                if (display) display.textContent = formatFn(val);
                
                // Specific update for compass
                if (id === 'axis') {
                    const needle = document.getElementById('compass-needle');
                    if (needle) needle.style.transform = `rotate(${val}deg)`;
                }
                
                this.processAndDraw();
            });
        };

        bindSlider('sphere', 'sphere', v => `${v >= 0 ? '+' : ''}${v.toFixed(2)} D`);
        bindSlider('cylinder', 'cylinder', v => `${v >= 0 ? '+' : ''}${v.toFixed(2)} D`);
        bindSlider('axis', 'axis', v => `${v}°`);
        bindSlider('distance', 'distance', v => `${v} cm`);
        bindSlider('skew-x', 'skewX', v => `${v.toFixed(1)}°`);
        bindSlider('skew-y', 'skewY', v => `${v.toFixed(1)}°`);
        bindSlider('anamorphic', 'anamorphic', v => `${v.toFixed(2)}x`);
        bindSlider('deconv', 'deconv', v => `${v.toFixed(1)}x`);
        bindSlider('contrast', 'contrast', v => `${v.toFixed(2)}x`);

        // Reset Button
        document.getElementById('btn-reset').addEventListener('click', () => {
            this.resetParams();
        });

        // Tabs
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                const target = e.currentTarget;
                target.classList.add('active');
                this.params.sourceType = target.dataset.source;

                // Toggle Config Bars
                const textBar = document.getElementById('text-config-bar');
                const imageBar = document.getElementById('image-config-bar');
                if (this.params.sourceType === 'text') {
                    textBar.classList.remove('hidden');
                    imageBar.classList.add('hidden');
                } else if (this.params.sourceType === 'image') {
                    textBar.classList.add('hidden');
                    imageBar.classList.remove('hidden');
                } else {
                    textBar.classList.add('hidden');
                    imageBar.classList.add('hidden');
                }

                this.renderSource();
                this.processAndDraw();
            });
        });

        // Mode buttons
        const modeMap = {
            'mode-precomp': 'precomp',
            'mode-split': 'split',
            'mode-original': 'original'
        };
        Object.entries(modeMap).forEach(([id, mode]) => {
            document.getElementById(id).addEventListener('click', (e) => {
                document.querySelectorAll('.toggle-btn').forEach(b => b.classList.remove('active'));
                e.currentTarget.classList.add('active');
                this.params.viewMode = mode;

                const splitDivider = document.getElementById('split-divider');
                if (mode === 'split') {
                    splitDivider.classList.add('active');
                } else {
                    splitDivider.classList.remove('active');
                }
                this.processAndDraw();
            });
        });

        // Custom text
        const textInput = document.getElementById('custom-text-input');
        textInput.addEventListener('input', (e) => {
            this.params.customText = e.target.value;
            this.renderSource();
            this.processAndDraw();
        });

        // Font size
        document.getElementById('font-size-select').addEventListener('change', (e) => {
            this.params.fontSize = parseInt(e.target.value);
            this.renderSource();
            this.processAndDraw();
        });

        // Image upload
        document.getElementById('image-upload').addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = (event) => {
                    const img = new Image();
                    img.onload = () => {
                        this.params.customImage = img;
                        this.params.currentImagePreset = 'custom';
                        document.querySelectorAll('.preset-img-btn').forEach(b => b.classList.remove('active'));
                        this.renderSource();
                        this.processAndDraw();
                    };
                    img.src = event.target.result;
                };
                reader.readAsDataURL(file);
            }
        });

        // Preset image buttons
        document.querySelectorAll('.preset-img-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.preset-img-btn').forEach(b => b.classList.remove('active'));
                e.currentTarget.classList.add('active');
                this.params.currentImagePreset = e.currentTarget.dataset.img;
                this.params.customImage = null;
                this.renderSource();
                this.processAndDraw();
            });
        });

        // Split drag
        this.setupSplitSlider();
    }

    setupSplitSlider() {
        const stage = document.getElementById('canvas-stage');
        const divider = document.getElementById('split-divider');
        let isDragging = false;

        const updateSplit = (clientX) => {
            const rect = this.canvas.getBoundingClientRect();
            let pos = (clientX - rect.left) / rect.width;
            pos = Math.max(0.05, Math.min(0.95, pos));
            this.params.splitPos = pos;
            divider.style.left = `${pos * 100}%`;
            this.processAndDraw();
        };

        divider.addEventListener('mousedown', (e) => {
            isDragging = true;
            e.preventDefault();
        });

        window.addEventListener('mousemove', (e) => {
            if (isDragging) {
                updateSplit(e.clientX);
            }
        });

        window.addEventListener('mouseup', () => {
            isDragging = false;
        });

        // Touch support
        divider.addEventListener('touchstart', () => { isDragging = true; });
        window.addEventListener('touchmove', (e) => {
            if (isDragging && e.touches[0]) {
                updateSplit(e.touches[0].clientX);
            }
        });
        window.addEventListener('touchend', () => { isDragging = false; });
    }

    resetParams() {
        this.params.sphere = -2.00;
        this.params.cylinder = -1.50;
        this.params.axis = 90;
        this.params.distance = 60;
        this.params.skewX = 0;
        this.params.skewY = 0;
        this.params.anamorphic = 1.08;
        this.params.deconv = 1.4;
        this.params.contrast = 1.25;

        const setVal = (id, val, str) => {
            const el = document.getElementById(id);
            if (el) el.value = val;
            const d = document.getElementById(`val-${id}`);
            if (d) d.textContent = str;
        };

        setVal('sphere', -2.00, '-2.00 D');
        setVal('cylinder', -1.50, '-1.50 D');
        setVal('axis', 90, '90°');
        setVal('distance', 60, '60 cm');
        setVal('skew-x', 0, '0.0°');
        setVal('skew-y', 0, '0.0°');
        setVal('anamorphic', 1.08, '1.08x');
        setVal('deconv', 1.4, '1.4x');
        setVal('contrast', 1.25, '1.25x');

        document.getElementById('compass-needle').style.transform = 'rotate(90deg)';
        this.processAndDraw();
    }

    // Render original sources to sourceCanvas
    renderSource() {
        const ctx = this.sourceCtx;
        const w = this.width;
        const h = this.height;

        ctx.fillStyle = '#0f172a';
        ctx.fillRect(0, 0, w, h);

        if (this.params.sourceType === 'text') {
            this.renderTextPattern(ctx, w, h);
        } else if (this.params.sourceType === 'chart') {
            this.renderSnellenChart(ctx, w, h);
        } else if (this.params.sourceType === 'image') {
            this.renderImagePattern(ctx, w, h);
        }
    }

    renderTextPattern(ctx, w, h) {
        // Card background
        ctx.fillStyle = '#1e293b';
        ctx.roundRect(40, 40, w - 80, h - 80, 16);
        ctx.fill();
        ctx.strokeStyle = '#334155';
        ctx.lineWidth = 2;
        ctx.stroke();

        // Title
        ctx.fillStyle = '#38bdf8';
        ctx.font = '700 24px "Outfit", sans-serif';
        ctx.fillText('PRUEBA DE AGUDEZA VISUAL Y LECTURA', 70, 90);

        ctx.fillStyle = '#94a3b8';
        ctx.font = '500 14px "JetBrains Mono", monospace';
        ctx.fillText('Test Pattern: Monospace, Serif & Sans-Serif Micro-Typography', 70, 115);

        // Separator
        ctx.strokeStyle = '#334155';
        ctx.beginPath();
        ctx.moveTo(70, 130);
        ctx.lineTo(w - 70, 130);
        ctx.stroke();

        // User Custom Text
        ctx.fillStyle = '#ffffff';
        const fontSize = this.params.fontSize;
        ctx.font = `600 ${fontSize}px "Outfit", sans-serif`;
        
        // Wrap text
        const text = this.params.customText;
        this.wrapText(ctx, text, 70, 170, w - 140, fontSize * 1.5);

        // Technical reading block
        const startY = 270;
        ctx.fillStyle = '#e2e8f0';
        ctx.font = '400 16px "Outfit", sans-serif';
        const samplePassage = 
            "El astigmatismo corneal genera una distorsión meridional que deforma los caracteres alfanuméricos. " +
            "Al aplicar una transformación afín inversa combinada con realce direccional de frecuencias espaciales, " +
            "las ondas de luz recibidas por la retina se aproximan a una forma nítida.";
        this.wrapText(ctx, samplePassage, 70, startY, w - 140, 24);

        // Code Box (Fine detail)
        ctx.fillStyle = '#090d16';
        ctx.roundRect(70, 360, w - 140, 200, 10);
        ctx.fill();
        ctx.strokeStyle = '#1e293b';
        ctx.stroke();

        ctx.fillStyle = '#10b981';
        ctx.font = '500 14px "JetBrains Mono", monospace';
        ctx.fillText('// Shader de Transformación Óptica & Deconvolución', 90, 395);

        ctx.fillStyle = '#c084fc';
        ctx.fillText('vec2', 90, 425);
        ctx.fillStyle = '#f8fafc';
        ctx.fillText(' compensateAstigmatism(vec2 uv, float cyl, float axisRad) {', 130, 425);

        ctx.fillStyle = '#94a3b8';
        ctx.fillText('    mat2 rot = mat2(cos(axisRad), -sin(axisRad), sin(axisRad), cos(axisRad));', 110, 450);
        ctx.fillText('    vec2 transformed = rot * uv;', 110, 475);
        ctx.fillText('    transformed.x *= (1.0 + cyl * 0.05); // Skew anamórfico inverso', 110, 500);
        
        ctx.fillStyle = '#38bdf8';
        ctx.fillText('    return transpose(rot) * transformed;', 110, 525);
        ctx.fillStyle = '#f8fafc';
        ctx.fillText('}', 90, 545);
    }

    renderSnellenChart(ctx, w, h) {
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(40, 40, w - 80, h - 80);

        ctx.fillStyle = '#000000';
        ctx.textAlign = 'center';

        // 20/200
        ctx.font = '900 80px "JetBrains Mono", sans-serif';
        ctx.fillText('E', w / 2, 130);

        // 20/100
        ctx.font = '900 50px "JetBrains Mono", sans-serif';
        ctx.fillText('F  P', w / 2, 195);

        // 20/70
        ctx.font = '900 38px "JetBrains Mono", sans-serif';
        ctx.fillText('T  O  Z', w / 2, 250);

        // 20/50
        ctx.font = '900 28px "JetBrains Mono", sans-serif';
        ctx.fillText('L  P  E  D', w / 2, 300);

        // 20/40
        ctx.font = '900 22px "JetBrains Mono", sans-serif';
        ctx.fillText('P  E  C  F  D', w / 2, 345);

        // 20/30
        ctx.font = '900 17px "JetBrains Mono", sans-serif';
        ctx.fillText('E  D  F  C  Z  P', w / 2, 385);

        // 20/20
        ctx.font = '900 13px "JetBrains Mono", sans-serif';
        ctx.fillText('F  E  L  O  P  Z  D', w / 2, 420);

        // Astigmatism Star / Radial Dial (Sunburst chart)
        const cx = w / 2;
        const cy = 520;
        const radius = 65;
        ctx.lineWidth = 2;
        ctx.strokeStyle = '#000000';

        for (let a = 0; a < 180; a += 15) {
            const rad = (a * Math.PI) / 180;
            const x1 = cx + Math.cos(rad) * radius;
            const y1 = cy + Math.sin(rad) * radius;
            const x2 = cx - Math.cos(rad) * radius;
            const y2 = cy - Math.sin(rad) * radius;
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();

            // Degree marks
            ctx.font = '600 9px sans-serif';
            ctx.fillText(`${a}°`, cx + Math.cos(rad) * (radius + 12), cy + Math.sin(rad) * (radius + 12));
        }

        ctx.textAlign = 'left'; // reset
    }

    renderImagePattern(ctx, w, h) {
        if (this.params.customImage) {
            ctx.drawImage(this.params.customImage, 0, 0, w, h);
            return;
        }

        const preset = this.params.currentImagePreset;
        if (preset === 'code') {
            // High detail IDE Mockup
            ctx.fillStyle = '#181824';
            ctx.fillRect(0, 0, w, h);

            // Window Header
            ctx.fillStyle = '#222233';
            ctx.fillRect(0, 0, w, 40);
            ctx.fillStyle = '#ef4444'; ctx.beginPath(); ctx.arc(20, 20, 6, 0, Math.PI * 2); ctx.fill();
            ctx.fillStyle = '#f59e0b'; ctx.beginPath(); ctx.arc(38, 20, 6, 0, Math.PI * 2); ctx.fill();
            ctx.fillStyle = '#10b981'; ctx.beginPath(); ctx.arc(56, 20, 6, 0, Math.PI * 2); ctx.fill();

            // Code Content
            ctx.fillStyle = '#cbd5e1';
            ctx.font = '500 15px "JetBrains Mono", monospace';
            const lines = [
                '// Optical Distortion Correction Test Bench',
                'const refractionParams = {',
                '    diopters_sphere: -2.75,',
                '    diopters_cylinder: -1.25,',
                '    meridian_angle_deg: 90,',
                '    vertex_distance_mm: 12.5,',
                '    focal_depth_cm: 65.0',
                '};',
                '',
                'function calculateOpticalTransform(eye, screenDist) {',
                '    const effectivePower = eye.diopters_sphere / (1.0 - (eye.vertex_distance_mm / 1000.0) * eye.diopters_sphere);',
                '    const astigmatismShear = Math.tan(eye.meridian_angle_deg * Math.PI / 180.0) * eye.diopters_cylinder;',
                '    return new Matrix3x3().rotate(eye.meridian_angle_deg).scale(1.0, 1.0 + astigmatismShear);',
                '}',
                '',
                'console.log("Compensating screen pixels for eyeglasses prescription...");'
            ];

            lines.forEach((line, idx) => {
                ctx.fillStyle = line.startsWith('//') ? '#64748b' : line.includes('function') || line.includes('const') ? '#818cf8' : '#f1f5f9';
                ctx.fillText(`${(idx + 1).toString().padStart(2, ' ')}  ${line}`, 30, 80 + idx * 26);
            });
        } else if (preset === 'chart') {
            // High frequency grid and gradient
            const grad = ctx.createLinearGradient(0, 0, w, h);
            grad.addColorStop(0, '#0f172a');
            grad.addColorStop(0.5, '#1e1b4b');
            grad.addColorStop(1, '#0284c7');
            ctx.fillStyle = grad;
            ctx.fillRect(0, 0, w, h);

            // Precision Grid
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)';
            ctx.lineWidth = 1;
            for (let x = 0; x < w; x += 25) {
                ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
            }
            for (let y = 0; y < h; y += 25) {
                ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
            }

            // Concentric circles for astigmatism distortion check
            ctx.strokeStyle = '#38bdf8';
            ctx.lineWidth = 2;
            for (let r = 30; r < 240; r += 30) {
                ctx.beginPath();
                ctx.arc(w / 2, h / 2, r, 0, Math.PI * 2);
                ctx.stroke();
            }
        } else if (preset === 'ui') {
            // Dashboard UI Mockup
            ctx.fillStyle = '#0f172a';
            ctx.fillRect(0, 0, w, h);

            ctx.fillStyle = '#1e293b';
            ctx.roundRect(30, 30, w - 60, 100, 12);
            ctx.fill();

            ctx.fillStyle = '#38bdf8';
            ctx.font = '700 22px "Outfit", sans-serif';
            ctx.fillText('SISTEMA DE CORRECCIÓN VISUAL ACTIVO', 50, 70);
            ctx.fillStyle = '#94a3b8';
            ctx.font = '400 14px "Outfit", sans-serif';
            ctx.fillText('Prescripción adaptativa sincronizada con el perfil del usuario', 50, 95);

            // 3 Stat Cards
            const cardW = (w - 60 - 30) / 3;
            for (let i = 0; i < 3; i++) {
                const cx = 30 + i * (cardW + 15);
                ctx.fillStyle = '#1e293b';
                ctx.roundRect(cx, 150, cardW, 140, 12);
                ctx.fill();

                ctx.fillStyle = '#64748b';
                ctx.font = '500 13px "Outfit", sans-serif';
                ctx.fillText(['DIOPTRÍAS EFECTIVAS', 'EJE DE MERIDIANO', 'RESOLUCIÓN RETINIANA'][i], cx + 20, 185);

                ctx.fillStyle = '#ffffff';
                ctx.font = '700 28px "JetBrains Mono", monospace';
                ctx.fillText(['-2.75 D', '90.0°', '20/20 HD'][i], cx + 20, 230);
            }
        }
    }

    wrapText(ctx, text, x, y, maxWidth, lineHeight) {
        const words = text.split(' ');
        let line = '';
        let currentY = y;

        for (let n = 0; n < words.length; n++) {
            const testLine = line + words[n] + ' ';
            const metrics = ctx.measureText(testLine);
            const testWidth = metrics.width;
            if (testWidth > maxWidth && n > 0) {
                ctx.fillText(line, x, currentY);
                line = words[n] + ' ';
                currentY += lineHeight;
            } else {
                line = testLine;
            }
        }
        ctx.fillText(line, x, currentY);
    }

    // Apply Optical Math & Transformation to Canvas
    processAndDraw() {
        const w = this.width;
        const h = this.height;
        const p = this.params;

        // 1. Clear processed canvas
        this.processedCtx.save();
        this.processedCtx.fillStyle = '#0b0f19';
        this.processedCtx.fillRect(0, 0, w, h);

        // Center origin for rotation and skew
        const cx = w / 2;
        const cy = h / 2;
        this.processedCtx.translate(cx, cy);

        // Calculate optical shear & anamorphic scaling based on Cylinder and Axis
        // Angle in radians
        const axisRad = (p.axis * Math.PI) / 180;
        
        // Distance scaling factor (further away = less effective cylinder power)
        const distFactor = 60.0 / Math.max(30, p.distance);

        // Skew angles in radians
        const skewXRad = (p.skewX * Math.PI) / 180;
        const skewYRad = (p.skewY * Math.PI) / 180;

        // Astigmatism anamorphic compensation factor
        const cylinderCompensation = 1.0 + (p.cylinder * -0.04 * distFactor);
        const totalAnamorphic = p.anamorphic * cylinderCompensation;

        // Transform matrix:
        // Rotate to meridian -> Scale / Skew along meridian -> Rotate back -> Apply manual skew
        this.processedCtx.rotate(axisRad);
        this.processedCtx.scale(totalAnamorphic, 1.0 / totalAnamorphic);
        this.processedCtx.transform(1, Math.tan(skewYRad), Math.tan(skewXRad), 1, 0, 0);
        this.processedCtx.rotate(-axisRad);

        // Draw original centered
        this.processedCtx.drawImage(this.sourceCanvas, -cx, -cy);
        this.processedCtx.restore();

        // 2. High-Pass Spatial Frequency Filter (Inverse PSF / Unsharp Masking) & Contrast
        this.applyDeconvolutionAndContrast();

        // 3. Final Composite onto display canvas
        this.ctx.clearRect(0, 0, w, h);

        if (p.viewMode === 'original') {
            this.ctx.drawImage(this.sourceCanvas, 0, 0);
        } else if (p.viewMode === 'precomp') {
            this.ctx.drawImage(this.processedCanvas, 0, 0);
        } else if (p.viewMode === 'split') {
            // Draw Pre-compensated on Left
            const splitX = w * p.splitPos;

            this.ctx.drawImage(this.processedCanvas, 0, 0);

            // Clip and draw Original on Right
            this.ctx.save();
            this.ctx.beginPath();
            this.ctx.rect(splitX, 0, w - splitX, h);
            this.ctx.clip();
            this.ctx.drawImage(this.sourceCanvas, 0, 0);
            this.ctx.restore();

            // Label Badges
            this.drawSplitLabels(splitX);
        }
    }

    applyDeconvolutionAndContrast() {
        const w = this.width;
        const h = this.height;
        const p = this.params;

        if (p.deconv === 0 && p.contrast === 1.0) return;

        const imgData = this.processedCtx.getImageData(0, 0, w, h);
        const data = imgData.data;
        const copy = new Uint8ClampedArray(data);

        const strength = p.deconv;
        const contrast = p.contrast;
        const factor = (259 * (contrast * 255 + 255)) / (255 * (259 - contrast * 255));

        // Laplacian edge kernel for inverse deconvolution along meridian
        const angleRad = (p.axis * Math.PI) / 180;
        const dx = Math.round(Math.cos(angleRad));
        const dy = Math.round(Math.sin(angleRad));

        for (let y = 1; y < h - 1; y++) {
            for (let x = 1; x < w - 1; x++) {
                const idx = (y * w + x) * 4;
                const idxN1 = ((y + dy) * w + (x + dx)) * 4;
                const idxN2 = ((y - dy) * w + (x - dx)) * 4;

                for (let c = 0; c < 3; c++) {
                    const center = copy[idx + c];
                    const neighbor1 = copy[idxN1 + c];
                    const neighbor2 = copy[idxN2 + c];

                    // Directional Laplacian high-pass
                    const highPass = center * 2 - neighbor1 - neighbor2;
                    let val = center + highPass * strength * 0.35;

                    // Contrast enhancement
                    if (contrast !== 1.0) {
                        val = factor * (val - 128) + 128;
                    }

                    data[idx + c] = Math.min(255, Math.max(0, val));
                }
            }
        }

        this.processedCtx.putImageData(imgData, 0, 0);
    }

    drawSplitLabels(splitX) {
        this.ctx.font = '600 12px "Outfit", sans-serif';
        
        // Left Label (Pre-Compensado)
        if (splitX > 120) {
            this.ctx.fillStyle = 'rgba(6, 182, 212, 0.85)';
            this.ctx.roundRect(16, 16, 140, 26, 6);
            this.ctx.fill();
            this.ctx.fillStyle = '#ffffff';
            this.ctx.fillText('PRE-COMPENSADO', 28, 33);
        }

        // Right Label (Original)
        if (this.width - splitX > 100) {
            this.ctx.fillStyle = 'rgba(30, 41, 59, 0.85)';
            this.ctx.roundRect(this.width - 116, 16, 100, 26, 6);
            this.ctx.fill();
            this.ctx.fillStyle = '#94a3b8';
            this.ctx.fillText('ORIGINAL', this.width - 98, 33);
        }
    }
}

// Instantiate engine when DOM is ready
window.addEventListener('DOMContentLoaded', () => {
    window.opticalEngine = new OpticalEngine();
});
