#pragma once

// API de soporte para tests: contador de llamadas a funciones stub.
int RaylibStub_GetLoadTextureCalls();
int RaylibStub_GetUnloadTextureCalls();
void RaylibStub_ResetCounters();
