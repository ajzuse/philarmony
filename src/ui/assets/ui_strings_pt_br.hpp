/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>

namespace filament_dryer {
namespace ui_strings_pt_br {

struct Entry {
    const char* key;
    const char* value;
};

inline constexpr Entry table[] = {
    {"btn_start", "Iniciar Secagem"},
    {"btn_stop", "Parar"},
    {"btn_pause", "Pausar"},
    {"btn_resume", "Retomar"},
    {"btn_confirm", "Confirmar"},
    {"btn_cancel", "Cancelar"},
    {"btn_more", "Mais"},
    {"btn_back", "Voltar"},
    {"btn_apply", "Aplicar"},
    {"btn_custom", "Personalizado"},
    {"btn_adjust", "Ajustar alvos"},
    {"btn_history", "Historico"},
    {"btn_settings", "Configuracoes"},
    {"btn_reconfigure", "Reconfigurar"},
    {"btn_calibrate", "Calibrar (4 pontos)"},
    {"btn_reset", "Restaurar configuracoes"},
    {"btn_export", "Enviar para App"},
    {"btn_keypad", "Teclado"},
    {"title_presets", "Perfis de filamento"},
    {"title_history", "Historico"},
    {"title_settings", "Configuracoes"},
    {"title_custom", "Ciclo personalizado"},
    {"title_drying", "Secando"},
    {"title_paused", "Pausado"},
    {"title_wifi", "WiFi"},
    {"title_display", "Tela"},
    {"title_units", "Unidades e idioma"},
    {"title_touch", "Toque"},
    {"title_advanced", "Avancado"},
    {"title_sensors", "Sensores"},
    {"title_keypad", "Teclado numerico"},
    {"title_confirm", "Confirmar"},
    {"title_adjust", "Ajustar alvos"},
    {"status_idle", "parado"},
    {"status_drying", "secando"},
    {"status_paused", "pausado"},
    {"status_stopped", "parado"},
    {"label_temp", "Temp"},
    {"label_rh", "UR"},
    {"label_elapsed", "Decorrido"},
    {"label_remaining", "Restante"},
    {"label_duration", "Duracao"},
    {"label_brightness", "Brilho"},
    {"label_timeout", "Timeout"},
    {"label_orientation", "Orientacao"},
    {"label_language", "Idioma"},
    {"label_contrast", "Alto contraste"},
    {"label_sensitivity", "Sensibilidade"},
    {"label_ssid", "SSID"},
    {"label_rssi", "Sinal"},
    {"label_firmware", "Firmware"},
    {"label_device", "Dispositivo"},
    {"label_safety", "Limite de seguranca"},
    {"label_no_history", "Nenhum ciclo concluido"},
    {"label_not_found", "Registro nao encontrado"},
    {"label_reason", "Motivo"},
    {"label_average", "Media"},
    {"label_max", "Max"},
    {"label_target", "Alvo"},
    {"label_status", "Estado"},
    {"sensors_readonly", "Sensores sao configurados pelo instalador."},
    {"wifi_hotspot_hint", "Inicie o hotspot para alterar a rede."},
    {"msg_stop", "Parar o ciclo ativo?"},
    {"msg_reset", "Restaurar configuracoes?"},
    {"toast_pause_timeout", "Tempo de pausa esgotado: ciclo parado"},
    {"toast_remote", "Estado alterado remotamente"},
    {"toast_invalid", "Alvos invalidos ou inseguros"},
    {"toast_start_fail", "Nao foi possivel iniciar"},
    {"toast_rejected", "Comando rejeitado"},
    {"toast_target_rejected", "Atualizacao rejeitada"},
    {"toast_hotspot", "Hotspot de configuracao solicitado"},
    {"toast_settings_rejected", "Configuracoes rejeitadas"},
    {"toast_reset", "Configuracoes restauradas"},
    {"toast_cal_ok", "Calibracao salva"},
    {"toast_cal_fail", "Falha na calibracao"},
    {"toast_export_ok", "Historico enviado"},
    {"toast_export_fail", "Falha ao exportar"},
    {"toast_timeout_saved", "Configuracoes salvas"},
    {"cal_top_left", "superior esquerdo"},
    {"cal_top_right", "superior direito"},
    {"cal_bottom_right", "inferior direito"},
    {"cal_bottom_left", "inferior esquerdo"},
    {"cal_prompt", "Toque no alvo: "},
};
inline constexpr size_t table_size = sizeof(table) / sizeof(table[0]);

}  // namespace ui_strings_pt_br
}  // namespace filament_dryer
