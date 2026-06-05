# -*- coding: utf-8 -*-
from pathlib import Path
import math

from PIL import Image, ImageDraw, ImageFont


OUT = Path(r"C:\Users\Admin\Documents\GitHub\SemiRaft_Project\outputs\parkour_system_flowchart.png")
OUT.parent.mkdir(parents=True, exist_ok=True)

W, H = 2400, 1500
img = Image.new("RGB", (W, H), "#F6F8FB")
d = ImageDraw.Draw(img)


def pick_font(size: int, bold: bool = False):
    candidates = []
    if bold:
        candidates.extend([
            r"C:\Windows\Fonts\malgunbd.ttf",
            r"C:\Windows\Fonts\malgunsl.ttf",
        ])
    candidates.extend([
        r"C:\Windows\Fonts\malgun.ttf",
        r"C:\Windows\Fonts\malgunbd.ttf",
        r"C:\Windows\Fonts\NanumGothic.ttf",
    ])
    for path in candidates:
        if Path(path).exists():
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


F_TITLE = pick_font(58, True)
F_SUB = pick_font(28)
F_PHASE = pick_font(34, True)
F_BOX_TITLE = pick_font(31, True)
F_BOX = pick_font(21)
F_SMALL = pick_font(22)
F_TAG = pick_font(21, True)

INK = "#172033"
MUTED = "#5B667A"
BLUE = "#2F6FED"
BLUE_D = "#1C4FAF"
BLUE_L = "#E8F0FF"
TEAL = "#008C95"
TEAL_D = "#00646B"
TEAL_L = "#E5F7F8"
GREEN = "#168A4A"
GREEN_L = "#E7F6ED"
RED = "#CB3A31"
RED_L = "#FDECEA"
AMBER = "#B46A00"
AMBER_L = "#FFF4DF"
PANEL = "#FFFFFF"
LINE = "#D8E0EC"
SHADOW = "#DCE4F2"
BG = "#F6F8FB"


def text_size(text, font):
    box = d.textbbox((0, 0), text, font=font)
    return box[2] - box[0], box[3] - box[1]


def wrap_text(text, font, max_w):
    lines = []
    for para in str(text).split("\n"):
        if not para:
            lines.append("")
            continue
        cur = ""
        chunks = []
        for chunk in para.split(" "):
            if text_size(chunk, font)[0] <= max_w:
                chunks.append(chunk)
            else:
                buf = ""
                for ch in chunk:
                    if text_size(buf + ch, font)[0] <= max_w:
                        buf += ch
                    else:
                        if buf:
                            chunks.append(buf)
                        buf = ch
                if buf:
                    chunks.append(buf)
        for chunk in chunks:
            test = chunk if not cur else cur + " " + chunk
            if text_size(test, font)[0] <= max_w:
                cur = test
            else:
                if cur:
                    lines.append(cur)
                cur = chunk
        if cur:
            lines.append(cur)
    return lines


def centered_text(cx, cy, text, font, fill=INK, max_w=9999, gap=7):
    lines = wrap_text(text, font, max_w)
    dims = [text_size(line, font) for line in lines]
    total_h = sum(h for _, h in dims) + max(0, len(lines) - 1) * gap
    y = cy - total_h / 2
    for line, (w, h) in zip(lines, dims):
        d.text((cx - w / 2, y), line, font=font, fill=fill)
        y += h + gap


def panel(x, y, w, h, title, color):
    d.rounded_rectangle([x + 9, y + 12, x + w + 9, y + h + 12], radius=26, fill=SHADOW)
    d.rounded_rectangle([x, y, x + w, y + h], radius=26, fill=PANEL, outline=LINE, width=2)
    d.text((x + 36, y + 30), title, font=F_PHASE, fill=color)


def rounded_rect(x, y, w, h, fill, outline, radius=18, width=3):
    d.rounded_rectangle([x + 6, y + 8, x + w + 6, y + h + 8], radius=radius, fill=SHADOW)
    d.rounded_rectangle([x, y, x + w, y + h], radius=radius, fill=fill, outline=outline, width=width)


def box(x, y, w, h, title, body="", fill=BLUE_L, outline=BLUE, title_color=BLUE_D):
    rounded_rect(x, y, w, h, fill, outline)
    if body:
        d.text((x + 28, y + 17), title, font=F_BOX_TITLE, fill=title_color)
        yy = y + 55
        for line in wrap_text(body, F_BOX, w - 56)[:3]:
            d.text((x + 28, yy), line, font=F_BOX, fill=INK)
            yy += 27
    else:
        centered_text(x + w / 2, y + h / 2, title, F_BOX_TITLE, title_color, w - 48)


def pill(x, y, w, h, text, fill, outline):
    rounded_rect(x, y, w, h, fill, outline, radius=h // 2)
    centered_text(x + w / 2, y + h / 2, text, F_BOX_TITLE, "white", w - 48)


def diamond(cx, cy, w, h, text):
    pts = [(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)]
    d.polygon([(x + 6, y + 8) for x, y in pts], fill=SHADOW)
    d.polygon(pts, fill=AMBER_L, outline=AMBER)
    d.line(pts + [pts[0]], fill=AMBER, width=3)
    centered_text(cx, cy, text, F_BOX_TITLE, INK, w - 90, 5)


def arrow(x1, y1, x2, y2, color="#52637A", width=4, label=None):
    d.line([x1, y1, x2, y2], fill=color, width=width)
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 17
    head = [
        (x2, y2),
        (x2 - size * math.cos(ang - math.pi / 6), y2 - size * math.sin(ang - math.pi / 6)),
        (x2 - size * math.cos(ang + math.pi / 6), y2 - size * math.sin(ang + math.pi / 6)),
    ]
    d.polygon(head, fill=color)
    if label:
        lx, ly = (x1 + x2) / 2, (y1 + y2) / 2
        tw, th = text_size(label, F_TAG)
        d.rounded_rectangle([lx - tw / 2 - 10, ly - th / 2 - 6, lx + tw / 2 + 10, ly + th / 2 + 6],
                            radius=12, fill=BG, outline=color, width=2)
        d.text((lx - tw / 2, ly - th / 2 - 1), label, font=F_TAG, fill=color)


def poly_arrow(points, color="#52637A", width=4, label=None, label_at=0):
    for p1, p2 in zip(points, points[1:]):
        d.line([p1, p2], fill=color, width=width)
    x1, y1 = points[-2]
    x2, y2 = points[-1]
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 17
    head = [
        (x2, y2),
        (x2 - size * math.cos(ang - math.pi / 6), y2 - size * math.sin(ang - math.pi / 6)),
        (x2 - size * math.cos(ang + math.pi / 6), y2 - size * math.sin(ang + math.pi / 6)),
    ]
    d.polygon(head, fill=color)
    if label:
        sx, sy = points[label_at]
        ex, ey = points[label_at + 1]
        lx, ly = (sx + ex) / 2, (sy + ey) / 2
        tw, th = text_size(label, F_TAG)
        d.rounded_rectangle([lx - tw / 2 - 10, ly - th / 2 - 6, lx + tw / 2 + 10, ly + th / 2 + 6],
                            radius=12, fill=BG, outline=color, width=2)
        d.text((lx - tw / 2, ly - th / 2 - 1), label, font=F_TAG, fill=color)


d.text((80, 42), "Player Parkour System Blueprint Flow", font=F_TITLE, fill=INK)
d.text((84, 114), "BP_MyFPC · Parkour 함수 + ParkourStart 이벤트 기반 요약 흐름도", font=F_SUB, fill=MUTED)

tag_x = 1365
tag_y = 58
for label, color in [
    ("SphereTraceSingleForObjects", BLUE),
    ("ObjectTypeQuery 1 / 2 / 8", TEAL),
    ("ParkourHeightBias = 40", AMBER),
    ("WaterBodyOcean Ignore", GREEN),
]:
    tw, th = text_size(label, F_SMALL)
    if tag_x + tw + 34 > W - 80:
        tag_x = 1365
        tag_y += 54
    d.rounded_rectangle([tag_x, tag_y, tag_x + tw + 34, tag_y + 44], radius=22, fill="#FFFFFF", outline=color, width=2)
    d.ellipse([tag_x + 12, tag_y + 15, tag_x + 24, tag_y + 27], fill=color)
    d.text((tag_x + 30, tag_y + 8), label, font=F_SMALL, fill=INK)
    tag_x += tw + 55

panel(90, 175, 1035, 1210, "1. Parkour 함수: 감지 / 판정", BLUE_D)
panel(1275, 175, 1035, 1210, "2. ParkourStart 이벤트: 이동 실행 / 복구", TEAL_D)

# Left side
xL, wL = 215, 785
pill(xL, 260, wL, 72, "Parkour 호출", BLUE, BLUE_D)
box(xL, 380, wL, 98, "기준값 캐싱", "Capsule Radius / Half Height / ForwardVector / Location", BLUE_L, BLUE)
box(xL, 525, wL, 108, "트레이스 설정", "ObjectChannelType, DebugVisibility=None\nActorIgnore에 WaterBodyOcean 추가", TEAL_L, TEAL, TEAL_D)
box(xL, 680, wL, 108, "전방 Sphere Trace", "전방 Start / End 계산\nSphereTraceSingleForObjects 실행", BLUE_L, BLUE)
diamond(607, 910, 470, 145, "Hit 발생?")
box(150, 895, 305, 82, "종료", "파쿠르 대상 없음", RED_L, RED, RED)
box(xL, 1060, wL, 112, "히트 정보 / 목표 계산", "BreakHitResult → HitActor 저장\nMiddleLocation / EndLocation 산출", GREEN_L, GREEN, GREEN)
box(xL, 1210, wL, 108, "상대 좌표 변환 후 호출", "ConvertRelativeParkourLocation\nParkourStart(RelativeStart, Middle, End, HitActor)", AMBER_L, AMBER, AMBER)

arrow(xL + wL / 2, 332, xL + wL / 2, 380, BLUE)
arrow(xL + wL / 2, 478, xL + wL / 2, 525, BLUE)
arrow(xL + wL / 2, 633, xL + wL / 2, 680, TEAL)
arrow(xL + wL / 2, 788, 607, 838, BLUE)
poly_arrow([(372, 910), (455, 910)], RED, label="False")
arrow(607, 982, xL + wL / 2, 1060, GREEN, label="True")
arrow(xL + wL / 2, 1172, xL + wL / 2, 1210, GREEN)

# Cross-panel call
poly_arrow([(1000, 1264), (1198, 1264), (1198, 260), (1375, 260)], TEAL, width=5)
pill(1012, 1228, 235, 74, "ParkourStart", TEAL, TEAL_D)

# Right side
xR, wR = 1400, 785
pill(xR, 260, wR, 72, "ParkourStart 이벤트 수신", TEAL, TEAL_D)
diamond(1792, 445, 470, 145, "bIsParkourRun?")
box(1980, 500, 235, 82, "Return", "이미 실행 중", RED_L, RED, RED)
box(xR, 595, wR, 96, "상태 잠금", "bIsParkourRun = true", TEAL_L, TEAL, TEAL_D)
box(xR, 730, wR, 112, "부모 / 충돌 처리", "K2_AttachToActor(HitActor), PrevCollision 저장\nCapsule NoCollision", BLUE_L, BLUE)
box(xR, 880, wR, 98, "이동 모드 전환", "CharacterMovement → MOVE_Flying", BLUE_L, BLUE)
box(xR, 1015, wR, 108, "Timeline 1", "ParkourTimeLinear: RelativeStart → RelativeMiddle\nCalculateParkourPos", GREEN_L, GREEN, GREEN)
box(xR, 1160, wR, 108, "Timeline 2", "RelativeMiddle → RelativeEnd / CalculateParkourPos", GREEN_L, GREEN, GREEN)
box(xR, 1300, wR, 96, "상태 복구", "MOVE_Walking, DetachFromActor, Collision 복구\nbIsParkourRun=false", TEAL_L, TEAL, TEAL_D)

arrow(xR + wR / 2, 332, 1792, 372, TEAL)
arrow(1792, 518, xR + wR / 2, 595, TEAL, label="False")
poly_arrow([(2027, 445), (2097, 445), (2097, 500)], RED, label="True")
arrow(xR + wR / 2, 691, xR + wR / 2, 730, TEAL)
arrow(xR + wR / 2, 842, xR + wR / 2, 880, BLUE)
arrow(xR + wR / 2, 978, xR + wR / 2, 1015, BLUE)
arrow(xR + wR / 2, 1123, xR + wR / 2, 1160, GREEN)
arrow(xR + wR / 2, 1268, xR + wR / 2, 1300, TEAL)

d.rounded_rectangle([90, 1422, 2310, 1472], radius=18, fill="#EDF2FA", outline=LINE, width=2)
footer = "핵심 변수: CapsuleRadius · CapsuleHalfHeight · CapsuleAxisHalfLength · MiddleLocation · EndLocation · HitActor · PrevCollisionEnableState"
centered_text(1200, 1447, footer, F_SMALL, MUTED, 2100)

img.save(OUT, "PNG")
print(OUT)
