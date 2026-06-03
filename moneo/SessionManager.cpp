#include "SessionManager.h"
#include "Config.h"

SessionManager::SessionManager()
    : _sessionNumber(0), _chunkIndex(0) {}

bool SessionManager::begin() {
    _sessionNumber = _detectNextSession();
    DLOGF("[Session] Next session number: %d\n", _sessionNumber);
    return true;
}

int SessionManager::_detectNextSession() {
    int n = 0;
    while (true) {
        String dir = "/session_" + String(n);
        if (!SD.exists(dir.c_str())) break;
        n++;
    }
    return n;
}

void SessionManager::startNewSession() {
    _chunkIndex = 0;
    _chunks.clear();
    _sessionDir = "/session_" + String(_sessionNumber);
    _createDirTree();
    _saveSessionMeta();
    DLOGF("[Session] Started session: %s\n", _sessionDir.c_str());
}

void SessionManager::_createDirTree() {
    SD.mkdir(_sessionDir.c_str());
    SD.mkdir((_sessionDir + "/raw").c_str());
    SD.mkdir((_sessionDir + "/notes").c_str());
    SD.mkdir((_sessionDir + "/transcripts").c_str());
    SD.mkdir((_sessionDir + "/meta").c_str());
}

void SessionManager::_saveSessionMeta() {
    String path = getMetaDir() + "/session.json";
    File f = SD.open(path.c_str(), FILE_WRITE);
    if (!f) return;

    StaticJsonDocument<256> doc;
    doc["session"]    = _sessionNumber;
    doc["started_ms"] = millis();
    doc["version"]    = "moneo-v2";
    serializeJson(doc, f);
    f.close();
}

String SessionManager::nextChunkPath() {
    char name[16];
    snprintf(name, sizeof(name), "%04d.wav", _chunkIndex + 1);

    ChunkState cs;
    cs.chunkIndex      = _chunkIndex + 1;
    cs.rawPath         = getRawDir() + "/" + String(name);
   String noteName = String(name);
noteName.replace(".wav", ".md");

String transcriptName = String(name);
transcriptName.replace(".wav", ".json");

cs.notePath = getNotesDir() + "/" + noteName;
cs.transcriptPath = getTranscriptsDir() + "/" + transcriptName;
    cs.uploadStatus    = "pending";
    cs.noteStatus      = "pending";
    cs.noteAvailable   = false;
    _chunks.push_back(cs);

    _chunkIndex++;
    return cs.rawPath;
}

ChunkState* SessionManager::getChunk(int index) {
    for (auto& c : _chunks) {
        if (c.chunkIndex == index) return &c;
    }
    return nullptr;
}

void SessionManager::markChunkStatus(int chunkIndex, const String& uploadStatus) {
    ChunkState* c = getChunk(chunkIndex);
    if (c) {
        c->uploadStatus = uploadStatus;
        saveUploadState();
    }
}

void SessionManager::markNoteReady(int chunkIndex) {
    ChunkState* c = getChunk(chunkIndex);
    if (c) {
        c->noteAvailable = true;
        c->noteStatus    = "completed";
        saveUploadState();
    }
}

void SessionManager::saveUploadState() {
    String path = getMetaDir() + "/upload_state.json";
    File f = SD.open(path.c_str(), FILE_WRITE);
    if (!f) return;

    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.createNestedArray("chunks");
    for (auto& c : _chunks) {
        JsonObject o       = arr.createNestedObject();
        o["index"]         = c.chunkIndex;
        o["raw"]           = c.rawPath;
        o["upload_status"] = c.uploadStatus;
        o["note_status"]   = c.noteStatus;
        o["note_available"]= c.noteAvailable;
    }
    serializeJson(doc, f);
    f.close();
}

void SessionManager::loadUploadState() {
    String path = getMetaDir() + "/upload_state.json";
    if (!SD.exists(path.c_str())) return;

    File f = SD.open(path.c_str(), FILE_READ);
    if (!f) return;

    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, f) != DeserializationError::Ok) {
        f.close();
        return;
    }
    f.close();

    JsonArray arr = doc["chunks"].as<JsonArray>();
    for (JsonObject o : arr) {
        ChunkState cs;
        cs.chunkIndex     = o["index"];
        cs.rawPath        = o["raw"].as<String>();
        cs.uploadStatus   = o["upload_status"].as<String>();
        cs.noteStatus     = o["note_status"].as<String>();
        cs.noteAvailable  = o["note_available"];
        _chunks.push_back(cs);
    }
}

void SessionManager::runRecovery() {
    DLOG("[Recovery] Scanning for incomplete sessions...");
    for (int s = 0; s < _sessionNumber; s++) {
        String dir = "/session_" + String(s) + "/meta/upload_state.json";
        if (SD.exists(dir.c_str())) {
            // Could load and re-queue failed uploads here
            logRecovery("Checked session " + String(s));
        }
    }
}

void SessionManager::logRecovery(const String& message) {
    String path = getMetaDir() + "/recovery.log";
    File f = SD.open(path.c_str(), FILE_APPEND);
    if (!f) return;
    f.print("[");
    f.print(millis());
    f.print("] ");
    f.println(message);
    f.close();
}
