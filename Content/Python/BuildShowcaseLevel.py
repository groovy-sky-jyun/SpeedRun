# -*- coding: utf-8 -*-
"""
대표 이미지용 파쿠르 쇼케이스 레벨 생성기.

에디터 Output Log 하단 콘솔을 "Python" 모드로 바꾼 뒤
    BuildShowcaseLevel.py
또는 "Cmd" 모드에서
    py "D:/UE5Project/SpeedRun/Content/Python/BuildShowcaseLevel.py"

주의: 실행하면 현재 열린 레벨을 닫고 새 레벨을 만든다. 미리 저장할 것.

배치 수치는 각 액션의 판정 조건에 맞춰져 있다 (캡슐 반지름 42 기준):
    Vault  : 50 < FrontHeight <= 150,  Depth <= 200
    Mantle : (FrontHeight <= 150 && Depth > 200) 또는 (150 < FrontHeight <= 250 && Depth >= 84)
    Hang   : 150 < FrontHeight <= 250. Depth < 84 이면 Mantle 이 거부해서 Hang 이 걸린다
"""

import unreal

LEVEL_PATH = "/Game/Maps/L_ParkourShowcase"

BP_PARKOUR_BLOCK = "/Game/BP_Obstacle/ParkourBlock"
BP_WALL_LEDGE = "/Game/BP_Obstacle/WallLedge"
BP_BAR = "/Game/BP_Obstacle/Bar"

SM_CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
MI_FLOOR = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
MI_DARK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"

EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log("[Showcase] " + msg)


def V(x, y, z):
    return unreal.Vector(float(x), float(y), float(z))


def R(pitch, yaw, roll):
    return unreal.Rotator(float(roll), float(pitch), float(yaw))


def spawn(cls, loc, rot=None, label=None):
    a = EAS.spawn_actor_from_class(cls, loc, rot or unreal.Rotator(0, 0, 0))
    if a and label:
        a.set_actor_label(label)
    return a


def load_bp(path):
    """블루프린트 제너레이티드 클래스. 없으면 None."""
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        log("!! 에셋 없음: " + path)
        return None
    return unreal.EditorAssetLibrary.load_blueprint_class(path)


def set_mobility(actor, mobility):
    rc = actor.root_component
    if rc:
        rc.set_editor_property("mobility", mobility)


# ---------------------------------------------------------------- 지오메트리

# SM_Cube 의 피벗이 최소 모서리인지 중앙인지 자동 판별한다.
# ParkourBlock 의 렛지 계산은 최소 모서리 기준이라 거기에 맞춘다.
_cube = unreal.EditorAssetLibrary.load_asset(SM_CUBE)
_b = _cube.get_bounds()
_CUBE_SIZE = V(_b.box_extent.x * 2.0, _b.box_extent.y * 2.0, _b.box_extent.z * 2.0)
_CUBE_MIN = V(_b.origin.x - _b.box_extent.x,
              _b.origin.y - _b.box_extent.y,
              _b.origin.z - _b.box_extent.z)


def spawn_box(loc, size, label, material=MI_FLOOR):
    """loc = 박스의 최소 모서리(-X,-Y,-Z), size = cm 단위 실제 크기."""
    scale = V(size.x / _CUBE_SIZE.x, size.y / _CUBE_SIZE.y, size.z / _CUBE_SIZE.z)
    # 피벗이 중앙이면 액터를 반 칸 밀어서 최소 모서리를 loc 에 맞춘다.
    origin = V(loc.x - _CUBE_MIN.x * scale.x,
               loc.y - _CUBE_MIN.y * scale.y,
               loc.z - _CUBE_MIN.z * scale.z)
    a = spawn(unreal.StaticMeshActor, origin, label=label)
    smc = a.static_mesh_component
    smc.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    smc.set_static_mesh(_cube)
    a.set_actor_scale3d(scale)
    mat = unreal.EditorAssetLibrary.load_asset(material)
    if mat:
        smc.set_material(0, mat)
    return a


def spawn_block(bp_class, loc, size, label):
    """AParkourBlock 파생. loc 는 액터 원점 = 블록의 최소 모서리."""
    if bp_class is None:
        return None
    a = spawn(bp_class, loc, label=label)
    # set_editor_property -> PostEditChangeProperty -> RerunConstructionScripts
    # 로 이어져서 스플라인 렛지가 새 크기에 맞게 다시 깔린다.
    a.set_editor_property("block_size", size)
    return a


# ---------------------------------------------------------------- 레벨 만들기

def build():
    log("SM_Cube size=%s localMin=%s" % (_CUBE_SIZE, _CUBE_MIN))

    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        log("기존 레벨을 덮어쓴다: " + LEVEL_PATH)
        unreal.EditorAssetLibrary.delete_asset(LEVEL_PATH)

    if not LES.new_level(LEVEL_PATH):
        unreal.log_error("[Showcase] 레벨 생성 실패. 현재 레벨을 먼저 저장했는지 확인.")
        return

    block = load_bp(BP_PARKOUR_BLOCK)

    # ---- 바닥 -------------------------------------------------------
    spawn_box(V(-1500, -2000, -100), V(8000, 4200, 100), "Floor", MI_FLOOR)

    # ---- 코스: +X 방향으로 진행 -------------------------------------
    # 1) Vault (낮고 얇음)             h=95   d=110  -> Vault
    spawn_block(block, V(700, -160, 0), V(110, 320, 95), "OBS_01_Vault_Low")

    # 2) Vault (콩볼트급)              h=130  d=190  -> Vault
    spawn_block(block, V(1250, -160, 0), V(190, 320, 130), "OBS_02_Vault_Kong")

    # 3) Mantle (낮지만 두꺼움)        h=140  d=360  -> Vault 거부, Mantle
    spawn_block(block, V(1850, -200, 0), V(360, 400, 140), "OBS_03_Mantle_LowThick")

    # 4) Mantle (높고 올라설 수 있음)  h=215  d=260  -> Mantle
    spawn_block(block, V(2650, -200, 0), V(260, 400, 215), "OBS_04_Mantle_High")

    # 5) Hang (높고 얇음)              h=235  d=55   -> Depth < 84 라 Mantle 거부, Hang
    spawn_block(block, V(3350, -350, 0), V(55, 700, 235), "OBS_05_Hang_ThinWall")
    # 매달린 뒤 넘어가서 딛는 자리
    spawn_block(block, V(3450, -350, 0), V(280, 700, 120), "OBS_05b_Hang_Landing")

    # 6) Shimmy: 높은 벽 + 앞으로 튀어나온 렛지 (렛지 상단 240 -> Hang 범위 안)
    spawn_box(V(4150, -500, 0), V(90, 1000, 620), "WALL_Shimmy", MI_DARK)
    ledge = load_bp(BP_WALL_LEDGE) or block
    spawn_block(ledge, V(4080, -500, 195), V(70, 1000, 45), "OBS_06_Shimmy_Ledge")

    # 7) 갭 점프: 350 간격
    spawn_block(block, V(4600, -300, 0), V(400, 600, 300), "OBS_07a_Gap_Near")
    spawn_block(block, V(5350, -300, 0), V(550, 600, 300), "OBS_07b_Gap_Far")

    # 8) 정밀 착지용 바 두 개 (에셋 있으면)
    bar = load_bp(BP_BAR)
    if bar:
        spawn_block(bar, V(4650, 250, 0), V(90, 90, 230), "OBS_08a_Precision_Bar")
        spawn_block(bar, V(5050, 250, 0), V(90, 90, 230), "OBS_08b_Precision_Bar")

    # ---- 배경 빌딩 (실루엣용, +Y 쪽에만 두어 카메라를 안 가린다) ----
    buildings = [
        (-1000, 900, 700, 700, 1400),
        (500, 1300, 600, 900, 2200),
        (1900, 1000, 800, 700, 1000),
        (3100, 1400, 700, 800, 1800),
        (4500, 1000, 900, 900, 1250),
        (5900, 1500, 800, 700, 2000),
    ]
    for i, (x, y, sx, sy, sz) in enumerate(buildings):
        spawn_box(V(x, y, 0), V(sx, sy, sz),
                  "BG_Building_%02d" % (i + 1),
                  MI_DARK if i % 2 else MI_FLOOR)

    # ---- 라이팅 / 분위기 --------------------------------------------
    sun = spawn(unreal.DirectionalLight, V(0, 0, 2000), R(-32, 125, 0), "Sun")
    set_mobility(sun, unreal.ComponentMobility.MOVABLE)
    dlc = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if dlc:
        dlc.set_editor_property("intensity", 8.0)
        dlc.set_editor_property("use_temperature", True)
        dlc.set_editor_property("temperature", 6200.0)

    sky = spawn(unreal.SkyLight, V(0, 0, 1200), label="SkyLight")
    set_mobility(sky, unreal.ComponentMobility.MOVABLE)
    slc = sky.get_component_by_class(unreal.SkyLightComponent)
    if slc:
        slc.set_editor_property("real_time_capture", True)
        slc.set_editor_property("intensity", 1.0)

    spawn(unreal.SkyAtmosphere, V(0, 0, 0), label="SkyAtmosphere")
    spawn(unreal.VolumetricCloud, V(0, 0, 0), label="VolumetricCloud")

    fog = spawn(unreal.ExponentialHeightFog, V(0, 0, -400), label="HeightFog")
    ehc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    if ehc:
        ehc.set_editor_property("fog_density", 0.012)
        ehc.set_editor_property("volumetric_fog", True)

    ppv = spawn(unreal.PostProcessVolume, V(0, 0, 0), label="PostProcess_Global")
    ppv.set_editor_property("unbound", True)
    s = ppv.get_editor_property("settings")
    s.set_editor_property("override_bloom_intensity", True)
    s.set_editor_property("bloom_intensity", 0.9)
    ppv.set_editor_property("settings", s)

    # ---- 플레이어 / 히어로 카메라 -----------------------------------
    spawn(unreal.PlayerStart, V(0, 0, 120), R(0, 0, 0), "PlayerStart")

    cam_loc = V(-1250, -2500, 1550)
    cam_rot = unreal.MathLibrary.find_look_at_rotation(cam_loc, V(2500, -150, 200))
    cam = spawn(unreal.CameraActor, cam_loc, cam_rot, "HeroCamera_Screenshot")
    cc = cam.camera_component
    if cc:
        cc.set_editor_property("field_of_view", 62.0)

    LES.save_current_level()
    log("완료. 레벨: " + LEVEL_PATH)
    log("스크린샷: 아웃라이너에서 HeroCamera_Screenshot 선택 후 우클릭 -> Pilot.")


build()
