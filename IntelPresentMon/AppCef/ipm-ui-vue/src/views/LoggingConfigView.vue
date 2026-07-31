<!-- Copyright (C) 2022 Intel Corporation -->
<!-- SPDX-License-Identifier: MIT -->

<script setup lang="ts">
import { usePreferencesStore } from '@/stores/preferences';
import { Api } from '@/core/api';
import { Action } from '@/core/hotkey';
import HotkeyButton from '@/components/HotkeyButton.vue';
import { zhCN } from '@/locales/zh-CN';

const prefs = usePreferencesStore();
const etlCaptureDisabled = true;
const etlCaptureDisabledMessage = zhCN.logging.etlDisabled;
async function handleEtlCapture() {
    prefs.notifyEtlLoggingDisabled()
}
async function handleEtlExplore() {
    await Api.exploreEtls()
}
function getEtlToggleButtonName() {
    return etlCaptureDisabled
      ? zhCN.logging.etlDisabledButton
      : prefs.etlLogging
        ? zhCN.logging.finishEtl
        : zhCN.logging.startEtl;
}
</script>

<template>
  <div class="page-wrap">
    <h2 class="mt-5 ml-5 header-top">
      {{ zhCN.logging.title }}
    </h2>

    <v-card class="page-card">
        <v-alert type="info" variant="tonal" class="mt-5">
          {{ etlCaptureDisabledMessage }}
        </v-alert>

        <v-row class="mt-5">
            <v-col cols="3">
                {{ zhCN.logging.etlCaptureHotkey }}
                <p class="text-medium-emphasis text-caption mb-0">
                    {{ zhCN.logging.etlCaptureHotkeyHelp }}
                </p>
            </v-col>
            <v-col cols="9">
                <v-row>
                    <v-col cols="6">
                        <hotkey-button :action="Action.ToggleEtlLogging" :disabled="etlCaptureDisabled"></hotkey-button>
                    </v-col>
                </v-row>
            </v-col>
        </v-row>

        <v-row class="mt-5">
            <v-col cols="3">
                {{ zhCN.logging.captureEtl }}
                <p class="text-medium-emphasis text-caption mb-0">
                    {{ zhCN.logging.captureEtlHelp }}
                </p>
            </v-col>
            <v-col cols="9">
                <v-row>
                    <v-col cols="6">
                        <v-btn :disabled="etlCaptureDisabled" @click="handleEtlCapture">{{getEtlToggleButtonName()}}</v-btn>
                    </v-col>
                </v-row>
            </v-col>
        </v-row>

        <v-row class="mt-5">
            <v-col cols="3">
                {{ zhCN.logging.etlFolder }}
                <p class="text-medium-emphasis text-caption mb-0">
                    {{ zhCN.logging.etlFolderHelp }}
                </p>
            </v-col>
            <v-col cols="9">
                <v-row>
                    <v-col cols="6">
                        <v-btn @click="handleEtlExplore">{{ zhCN.common.openInExplorer }}</v-btn>
                    </v-col>
                </v-row>
            </v-col>
        </v-row>

    </v-card>
  </div>
</template>

<style scoped>
.top-label {
  margin: 0;
  padding: 0;
  height: auto;
}
.header-top {
  color: white;
  user-select: none;
}
.page-card {
  margin: 15px 0;
  padding: 0 15px 15px;
}
.page-wrap {
  max-width: 750px;
  flex-grow: 1;
}
</style>
