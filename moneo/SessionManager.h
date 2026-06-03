#ifndef SessionManager_h
#define SessionManager_h

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

// ============================================================
// SessionManager — Manages session directories, metadata,
// upload state, and crash recovery tracking.
//
// SD layout for each session:
//   /session_NNNN/raw/0001.wav
//   /session_NNNN/notes/0001.md
//   /session_NNNN/transcripts/0001.json
//   /session_NNNN/meta/session.json
//   /session_NNNN/meta/upload_state.json
//   /session_NNNN/meta/recovery.log
// ============================================================

struct ChunkState {
    int    chunkIndex;
    String rawPath;
    String notePath;
    String transcriptPath;

    // Upload states: pending | uploading | uploaded | processing | completed | failed
    String uploadStatus;
    String noteStatus;
    bool   noteAvailable;
};

class SessionManager {
public:
    SessionManager();
    bool begin();

    // Create a new session directory tree
    void startNewSession();

    // Return path for the next chunk WAV file
    String nextChunkPath();

    // Mark a chunk state (called by upload/sync tasks)
    void markChunkStatus(int chunkIndex, const String& uploadStatus);
    void markNoteReady(int chunkIndex);

    // Persist upload state to SD (called after status changes)
    void saveUploadState();

    // Load existing upload state (called on recovery)
    void loadUploadState();

    // Recovery: scan for incomplete WAVs and log them
    void runRecovery();

    // Write a line to recovery.log
    void logRecovery(const String& message);

    // Getters
    int    getSessionNumber()      const { return _sessionNumber; }
    int    getCurrentChunkIndex()  const { return _chunkIndex; }
    String getSessionDir()         const { return _sessionDir; }
    String getRawDir()             const { return _sessionDir + "/raw"; }
    String getNotesDir()           const { return _sessionDir + "/notes"; }
    String getTranscriptsDir()     const { return _sessionDir + "/transcripts"; }
    String getMetaDir()            const { return _sessionDir + "/meta"; }

    ChunkState* getChunk(int index);
    int         chunkCount() const { return _chunks.size(); }

private:
    void _createDirTree();
    void _saveSessionMeta();
    int  _detectNextSession();

    int    _sessionNumber;
    int    _chunkIndex;
    String _sessionDir;

    std::vector<ChunkState> _chunks;
};

#endif
