// ════════════════════════════════════════════════════════════════
//  Arpeggiator — sample-accurate, processBlock-driven
//  Ported from arp.js (Chris Wilson lookahead → block-based)
// ════════════════════════════════════════════════════════════════
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace mono101
{

// ── Event returned by process() ──────────────────────────────
struct ArpEvent
{
    int sampleOffset;   // sample index within the current block
    int midiNote;       // MIDI note number
    float velocity;     // 0.0–1.0
    bool isNoteOn;      // true = note-on, false = note-off
};

// ── Pattern enum ─────────────────────────────────────────────
enum class ArpPattern { Up, Down, UpDown, Random };

// ── Arpeggiator ──────────────────────────────────────────────
class Arpeggiator
{
public:
    Arpeggiator()
    {
        rng_.seed (std::random_device{}());
    }

    // ── Note input ───────────────────────────────────────────
    void noteOn (int midiNote, float velocity = 1.0f)
    {
        // avoid duplicates
        for (const auto& h : held_)
            if (h.note == midiNote) return;

        held_.push_back ({ midiNote, velocity });
        build();
        if (! running_)
            start();
    }

    void noteOff (int midiNote)
    {
        held_.erase (
            std::remove_if (held_.begin(), held_.end(),
                [midiNote](const HeldNote& h) { return h.note == midiNote; }),
            held_.end());
        build();
        if (held_.empty())
            stop();
    }

    // ── Parameter setters ────────────────────────────────────
    void setBPM (double bpm)               { bpm_ = bpm; }
    void setDivision (int div)             { division_ = div; }  // 8, 16, or 32
    void setPattern (ArpPattern pat)       { pattern_ = pat; build(); }
    void setOctaveRange (int octaves)      { octaves_ = octaves; build(); }  // 1 or 2

    // ── State queries ────────────────────────────────────────
    bool isRunning() const                 { return running_; }
    ArpPattern getPattern() const          { return pattern_; }
    int getDivision() const                { return division_; }
    double getBPM() const                  { return bpm_; }
    int getOctaveRange() const             { return octaves_; }

    // ── Process one audio block ──────────────────────────────
    //  Call once per processBlock.  Returns note-on/off events
    //  with sample-accurate offsets into the block.
    std::vector<ArpEvent> process (int numSamples, double sampleRate)
    {
        std::vector<ArpEvent> events;
        if (! running_ || seq_.empty())
            return events;

        const double stepSamples = stepSeconds() * sampleRate;
        const double gateSamples = stepSamples * gateRatio_;

        for (int i = 0; i < numSamples; ++i)
        {
            // ── note-off for the previous step ──────────────
            if (noteIsOn_ && sampleCounter_ >= noteOffSample_)
            {
                events.push_back ({ i, currentNote_, 0.0f, false });
                noteIsOn_ = false;
            }

            // ── note-on at the step boundary ────────────────
            if (sampleCounter_ >= nextStepSample_)
            {
                int idx;
                if (pattern_ == ArpPattern::Random)
                {
                    std::uniform_int_distribution<int> dist (0, (int) seq_.size() - 1);
                    idx = dist (rng_);
                }
                else
                {
                    idx = idx_;
                }

                currentNote_    = seq_[idx];
                currentVel_     = velocities_[idx];
                noteOffSample_  = nextStepSample_ + gateSamples;
                nextStepSample_ += stepSamples;

                // if a note was still on (gate > step — shouldn't happen
                // with 50% ratio, but guard anyway), send note-off first
                if (noteIsOn_)
                    events.push_back ({ i, currentNote_, 0.0f, false });

                events.push_back ({ i, currentNote_, currentVel_, true });
                noteIsOn_ = true;

                // advance index
                if (! seq_.empty())
                    idx_ = (idx_ + 1) % (int) seq_.size();
            }

            ++sampleCounter_;
        }

        return events;
    }

    // ── Start / Stop ─────────────────────────────────────────
    void start()
    {
        if (running_) return;
        running_        = true;
        idx_            = 0;
        sampleCounter_  = 0.0;
        nextStepSample_ = 0.0;
        noteOffSample_  = 0.0;
        noteIsOn_       = false;
    }

    void stop()
    {
        // Caller should handle any final note-off if noteIsOn_
        running_ = false;
        noteIsOn_ = false;
    }

    // If a note is currently sounding when stop() is called,
    // the caller can query this to send a final note-off.
    bool hasActiveNote() const    { return noteIsOn_; }
    int  getActiveNote() const    { return currentNote_; }

private:
    // ── Held-note record ─────────────────────────────────────
    struct HeldNote
    {
        int   note;
        float velocity;
    };

    // ── Build the sequence from held notes ───────────────────
    void build()
    {
        // sort held notes ascending
        auto sorted = held_;
        std::sort (sorted.begin(), sorted.end(),
            [](const HeldNote& a, const HeldNote& b) { return a.note < b.note; });

        // expand across octaves
        std::vector<int>   notes;
        std::vector<float> vels;
        for (int o = 0; o < octaves_; ++o)
        {
            for (const auto& h : sorted)
            {
                notes.push_back (h.note + o * 12);
                vels.push_back (h.velocity);
            }
        }

        switch (pattern_)
        {
            case ArpPattern::Up:
                seq_        = notes;
                velocities_ = vels;
                break;

            case ArpPattern::Down:
                seq_        = std::vector<int>   (notes.rbegin(), notes.rend());
                velocities_ = std::vector<float> (vels.rbegin(),  vels.rend());
                break;

            case ArpPattern::UpDown:
                if (notes.size() < 2)
                {
                    seq_        = notes;
                    velocities_ = vels;
                }
                else
                {
                    seq_        = notes;
                    velocities_ = vels;
                    // append reversed, excluding first and last to avoid doubles
                    for (int i = (int) notes.size() - 2; i >= 1; --i)
                    {
                        seq_.push_back (notes[i]);
                        velocities_.push_back (vels[i]);
                    }
                }
                break;

            case ArpPattern::Random:
                seq_        = notes;
                velocities_ = vels;
                break;
        }

        if (! seq_.empty())
            idx_ = idx_ % (int) seq_.size();
    }

    // ── Step duration in seconds ─────────────────────────────
    double stepSeconds() const
    {
        return (60.0 / bpm_) * (4.0 / division_);
    }

    // ── State ────────────────────────────────────────────────
    bool        running_    = false;
    ArpPattern  pattern_    = ArpPattern::Up;
    double      bpm_        = 120.0;
    int         division_   = 16;
    int         octaves_    = 1;
    double      gateRatio_  = 0.5;   // note-on duration as fraction of step

    int         idx_        = 0;
    std::vector<int>   seq_;
    std::vector<float> velocities_;

    std::vector<HeldNote> held_;

    // sample-accurate scheduling state
    double  sampleCounter_  = 0.0;
    double  nextStepSample_ = 0.0;
    double  noteOffSample_  = 0.0;
    bool    noteIsOn_       = false;
    int     currentNote_    = 0;
    float   currentVel_     = 1.0f;

    // RNG for random pattern
    std::mt19937 rng_;
};

} // namespace mono101
