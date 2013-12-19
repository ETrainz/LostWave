//  UI/MusicSelector.hpp :: Music selection UI component
//  Copyright 2013 Keigen Shu

#include "MusicSelector.hpp"

namespace UI {

// Constructor
MusicSelector::MusicSelector(clan::GUIComponent *parent, JSONReader &skin, MusicList const &list) :
    clan::GUIComponent(parent, { recti(0, 0, parent->get_size()), false }, "music_selector"),
    mMusicList(list),

    // List element starting offset
    mso (skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.base-offset",
                vec2i(16, 16))),

    // Offset per list element
    mAeo(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.active-element-offset",
                vec2i(0, 48))),
    mIeo(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.inactive-element-offset",
                vec2i(0, 32))),

    // List element artist offset
    mAao(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.active-artist-offset",
                vec2i(0, 8))),
    mIao(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.inactive-artist-offset",
                vec2i(0, 8))),

    // List element title offset
    mAto(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.active-title-offset",
                vec2i(0, 28))),
    mIto(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.inactive-title-offset",
                vec2i(0, 20))),

    // Selected element display offsets
    mSgo(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-genre-offset",
                vec2i(0, 20))),
    mSao(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-artist-offset",
                vec2i(0, 20))),
    mSto(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-title-offset",
                vec2i(0, 20))),
    mSno(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-notecharter-offset",
                vec2i(0, 20))),
    mSdo(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-difficulty-offset",
                vec2i(0, 20))),
    mSbo(skin.get_or_set(
                &JSONReader::getVec2i, "theme.music-selector.selected-tempo-offset",
                vec2i(0, 20))),

    mLdelay(skin.get_if_else_set(
                &JSONReader::getInteger, "player.scroll-delay", 0,
                [] (int const &value) -> bool { return value >= 0 && value <= 30; }
                )), // Scroll delay within 0 .. 30 frames, 5 frames delay as default
    mLcounter(0),
    mVcount(skin.get_if_else_set(
                &JSONReader::getInteger, "theme.music-selector.size", 10,
                [] (int const &value) -> bool { return value >  0 && value <= 20; }
                )), // Display up to 20 elements per screen, 10 elements as default
    mVtop(0),
    mVptr(0),
    mVprv  (-1),
    mVstore(-1)
    {
        clan::Canvas canvas = get_canvas();

        mAaf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.active-artist-font"));
        mIaf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.inactive-artist-font"));
        mAtf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.active-title-font"));
        mItf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.inactive-title-font"));
        mShf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.selected-head-font"));
        mSbf = clan::Font(canvas, skin.getFontDesc("theme.music-selector.selected-body-font"));

        set_constant_repaint(true);

        func_render().set(this, &MusicSelector::render);
        func_input ().set(this, &MusicSelector::process_input);
    }

Music* MusicSelector::get() const
{
    if (mVptr < 0)
        return nullptr;

    MusicList::const_iterator ip = mMusicList.begin(); // Selected item
    for(int i=0; i<mVptr; ip++, i++);

    // TODO Random music

    return *ip;
}

////    GUI Component Callbacks    ////////////////////////////////
bool MusicSelector::process_input(const clan::InputEvent& event)
{
    // dump_event(event, "MusicSelector");
    /****/ if (event.device.get_type() == clan::InputDevice::Type::keyboard)
    {
        /****/ if (event.type == clan::InputEvent::Type::pressed)
        {
            switch (event.id)
            {
                case clan::InputCode::keycode_up:
                    if (mLcounter <= 0) {
                        mVptr = (mVptr > 0) ? (mVptr - 1) : 0;
                        mLcounter = mLdelay;
                    } else {
                        mLcounter -= 1;
                    }
                    break;

                case clan::InputCode::keycode_down:
                    if (mLcounter <= 0) {
                        mVptr = (mVptr < static_cast<int>(mMusicList.size())) ? (mVptr + 1) : mMusicList.size();
                        mLcounter = mLdelay;
                    } else {
                        mLcounter -= 1;
                    }
                    break;

                default:
                    return false;
            }

            if((mVptr - mVtop) > (mVcount - 1))
                mVtop = mVptr  - (mVcount - 1);
            else if ((mVptr - mVtop) <= 0)
                mVtop = mVptr;

            return false;
        } else if (event.type == clan::InputEvent::Type::released) {
            switch (event.id)
            {
                case clan::InputCode::keycode_escape:
                    mVptr = mVstore;
                    exit_with_code(1);
                    return true;

                case clan::InputCode::keycode_enter:
                    mVstore = mVptr;
                    // TODO Select random song
                    exit_with_code(0);
                    return true;
            }

            mLcounter -= 1;
        }
    } else if (event.device.get_type() == clan::InputDevice::Type::pointer) {

    }

    return false;
}

void MusicSelector::render(clan::Canvas& canvas, const recti& clip_rect)
{
    // Top-of-screen and currently-pointed iterators
    MusicList::const_iterator it = mMusicList.cbegin(); // Top / current iteration
    MusicList::const_iterator ip = mMusicList.cbegin(); // Selected item
    MusicList::const_iterator il = mVprv == -1 ? mMusicList.cend() : mMusicList.cbegin(); // Previous item

    for(int i=0; i<mVtop; it++, i++);
    for(int i=0; i<mVptr; ip++, i++);
    for(int i=0; i<mVprv; il++, i++);

    if (ip == mMusicList.cend()) {
        // TODO Add random selection background image
        mBGImg = clan::Image();
    } else if (ip != il) {
        if (il != mMusicList.cend())
        (*il)->charts.begin()->second->setCoverArt();
        (*ip)->charts.begin()->second->load_art();

        mVprv = mVptr;

        clan::PixelBuffer &cvrart = (*ip)->charts.begin()->second->getCoverArt();
        if (cvrart.is_null()) {
            mBGImg = clan::Image();
        } else {
            mBGImg = clan::Image(canvas, cvrart, recti(0,0,800,600));
            mBGImg.set_alpha(0.333f);
        }
    }

    if (mBGImg.is_null() == false)
    {
        float scale = std::min(
            static_cast<float>(canvas.get_width ()) / static_cast<float>(mBGImg.get_width ()),
            static_cast<float>(canvas.get_height()) / static_cast<float>(mBGImg.get_height())
        );

        mBGImg.draw( canvas, alignCC(
            static_cast<sizef>(canvas.get_size()),
            static_cast<sizef>(mBGImg.get_size()) * scale
        ) );
    }

    point2f pos = mso;

    for(int i=0; i<mVcount; it++, i++)
    {
        if (it == mMusicList.end())
        {
            if (it == ip)
                mAtf.draw_text(canvas, pos + mAto, "RANDOM");
            else
                mItf.draw_text(canvas, pos + mIto, "Random");

            break;
        } else if (it == ip) {
            mAaf.draw_text(canvas, pos + mAao, (*ip)->artist);
            mAtf.draw_text(canvas, pos + mAto, (*ip)->title);
            pos += mAeo;

            mSbf.draw_text(canvas, mSgo, (*ip)->genre);
            mSbf.draw_text(canvas, mSao, (*ip)->artist);
            mShf.draw_text(canvas, mSto, (*ip)->title);
            mSbf.draw_text(canvas, mSno,
                    clan::string_format( "Charted by %1", (*ip)->charts[0]->getCharter())
                    );
            mSbf.draw_text(canvas, mSdo,
                    clan::string_format("Level %1", (*ip)->charts[0]->getLevel())
                    );
            {
                float const tempo = (*ip)->charts[0]->getTempo();
                clan::StringFormat f("%1.%2 BPM");
                f.set_arg(1, static_cast<int>( tempo ));
                f.set_arg(2, static_cast<int>((tempo - std::floor(tempo)) * 100.f), 2);

                mSbf.draw_text(canvas, mSbo, f.get_result());
            }

        } else {
            mIaf.draw_text(canvas, pos + mIao, (*it)->artist);
            mItf.draw_text(canvas, pos + mIto, (*it)->title);
            pos += mIeo;
        }
    }
}


}
