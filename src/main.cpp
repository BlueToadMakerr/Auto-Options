#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

bool g_enableObjectSpawn = false;

CCMenuItemSpriteExtra* g_toggleBtn = nullptr;

void placeCustomObject(PlayerObject* player, int holdState) {
    if (!g_enableObjectSpawn || !player || !player->m_editorEnabled)
        return;

    auto editor = LevelEditorLayer::get();
    if (!editor || editor->m_playbackMode != PlaybackMode::Playing)
        return;

    auto pos = player->getPosition();
    pos.y -= 90.f;

    int settingVal = Mod::get()->getSettingValue<int>("custom-setting-value");

    // Get the correct id for each player
    int playerNumber = 165;
    if (player->m_isSecondPlayer)
        playerNumber = 199;

    std::string objStr = fmt::format("1,2899,2,{},3,{},{},{}", pos.x, pos.y, playerNumber, holdState);

    if (settingVal > 0)
        objStr += fmt::format(",33,{}", settingVal);

    editor->createObjectsFromString(objStr.c_str(), false, false);
}

// Player Hook
class $modify(MyPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        auto ret = PlayerObject::pushButton(btn);

        if (!m_editorEnabled || !g_enableObjectSpawn)
            return ret;

        auto lel = LevelEditorLayer::get();
        if (!lel || lel->m_playbackMode != PlaybackMode::Playing)
            return ret;

        if (btn == PlayerButton::Jump) {
            placeCustomObject(this, -1);
        }

        return ret;
    }

    bool releaseButton(PlayerButton btn) {
        auto ret = PlayerObject::releaseButton(btn);

        if (!m_editorEnabled || !g_enableObjectSpawn)
            return ret;

        auto lel = LevelEditorLayer::get();
        if (!lel || lel->m_playbackMode != PlaybackMode::Playing)
            return ret;

        if (btn == PlayerButton::Jump) {
            placeCustomObject(this, 1);
        }

        return ret;
    }
};

// -----------------------------------------
// LevelEditorLayer visibility logic
// -----------------------------------------
class $modify(MyEditorLayer, LevelEditorLayer) {
    void onPlaytest() {
        if (g_toggleBtn) g_toggleBtn->setVisible(false);
        LevelEditorLayer::onPlaytest();
    }

    void onResumePlaytest() {
        if (g_toggleBtn) g_toggleBtn->setVisible(false);
        LevelEditorLayer::onResumePlaytest();
    }

    void onPausePlaytest() {
        if (g_toggleBtn) g_toggleBtn->setVisible(true);
        LevelEditorLayer::onPausePlaytest();
    }

    void onStopPlaytest() {
        if (g_toggleBtn) g_toggleBtn->setVisible(true);
        LevelEditorLayer::onStopPlaytest();
    }
};

// -----------------------------------------
// Editor UI (toggle button)
// -----------------------------------------
class $modify(MyEditorUI, EditorUI) {
    void updateButtonState() {
        if (!g_toggleBtn) return;

        auto spr = ButtonSprite::create("Auto\nOptions", 25, true,
            "bigFont.fnt", "GJ_button_01.png", 40.f, 0.6f);

        if (g_enableObjectSpawn)
            spr->setColor({255, 255, 255});
        else
            spr->setColor({100, 100, 100});

        g_toggleBtn->setNormalImage(spr);
    }

    void onToggleButton(CCObject*) {
        g_enableObjectSpawn = !g_enableObjectSpawn;
        updateButtonState();
    }

#ifndef GEODE_IS_ANDROID
    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);
        if (m_editorLayer->m_playbackMode != PlaybackMode::Paused)
            return;
        if (g_toggleBtn) g_toggleBtn->setVisible(true);
    }
#endif

    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor))
            return false;

        g_enableObjectSpawn = false;

        auto spr = ButtonSprite::create("Auto\nOptions", 25, true,
            "bigFont.fnt", "GJ_button_01.png", 40.f, 0.6f);
        spr->setColor({100, 100, 100});

        g_toggleBtn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(MyEditorUI::onToggleButton));

        if (m_playtestBtn && m_playtestBtn->getParent()) {
            g_toggleBtn->setPosition(m_playtestBtn->getPosition() + ccp(70.f, 0.f));
            m_playtestBtn->getParent()->addChild(g_toggleBtn);
        }

        return true;
    }
};