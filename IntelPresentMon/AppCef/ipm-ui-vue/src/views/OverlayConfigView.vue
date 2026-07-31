<!-- Copyright (C) 2022 Intel Corporation -->
<!-- SPDX-License-Identifier: MIT -->
  
<script setup lang="ts">
import { usePreferencesStore } from '@/stores/preferences';
import OverlayPositioner from '@/components/OverlayPositioner.vue';
import ColorPicker from '@/components/ColorPicker.vue';
import { zhCN } from '@/locales/zh-CN';
const prefs = usePreferencesStore();
</script>

<template>
<div class="page-wrap">
    
    <h2 class="mt-5 ml-5 header-top">
        {{ zhCN.overlay.title }}
    </h2>

    <v-card class="page-card">

        <v-row class="mt-5">
        <v-col cols="3">
            {{ zhCN.overlay.windowedMode }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.windowedModeHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-row>
                <v-col cols="6">
                    <v-switch v-model="prefs.preferences.independentWindow" :label="zhCN.common.enable"></v-switch>
                </v-col>
            </v-row>
        </v-col>
        </v-row>

        <v-row class="mt-8">
        <v-col cols="3">
            {{ zhCN.overlay.automaticHide }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.automaticHideHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-row>
                <v-col cols="6">
                    <v-switch v-model="prefs.preferences.hideDuringCapture" :label="zhCN.common.enable"></v-switch>
                </v-col>
            </v-row>
        </v-col>
        </v-row>

        <v-row class="mt-8">       
        <v-col cols="3">
            {{ zhCN.overlay.position }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.positionHelp }}</p>
        </v-col>
        <v-col cols="9">
            <overlay-positioner v-model="prefs.preferences.overlayPosition"></overlay-positioner>
        </v-col>
        </v-row>

        <v-row class="mt-8">
        <v-col cols="3">
            {{ zhCN.overlay.width }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.widthHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-row>
                <v-col cols="12">
                    <v-slider
                        v-model="prefs.preferences.overlayWidth"
                        :max="1920"
                        :min="200"
                        thumb-label="always"
                    ></v-slider>
                </v-col>
            </v-row>
        </v-col>
        </v-row>

        <v-row class="mt-8">
        <v-col cols="3">
            {{ zhCN.overlay.timeScale }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.timeScaleHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-slider
            v-model="prefs.preferences.timeRange"
            :max="10"
            :min="0.1"
            :step="0.1"
            thumb-label="always"
            ></v-slider>
        </v-col>
        </v-row>

        <v-row class="mt-8">
        <v-col cols="3">
            {{ zhCN.overlay.graphicsScaling }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.graphicsScalingHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-row>
                <v-col cols="4">
                    <v-switch v-model="prefs.preferences.upscale" :label="zhCN.common.enable"></v-switch>
                </v-col>
                <v-col cols="8">
                    <v-slider
                        class="mt-3"
                        :label="zhCN.overlay.factor"
                        v-model="prefs.preferences.upscaleFactor"
                        :max="5"
                        :min="1"
                        :step="0.1"
                        :disabled="!prefs.preferences.upscale"
                        thumb-label="always"
                    ></v-slider>
                </v-col>
            </v-row>
        </v-col>
        </v-row>

        <v-row class="mt-8">       
        <v-col cols="3">
            {{ zhCN.overlay.drawRate }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.drawRateHelp }}</p>
        </v-col>
        <v-col cols="9">
            <v-slider
                v-model="prefs.preferences.overlayDrawRate"
                :max="120"
                :min="1"
                thumb-label="always"
            ></v-slider>
        </v-col>
        </v-row>

        <v-row class="mt-8">       
        <v-col cols="3">
            {{ zhCN.overlay.backgroundColor }}
            <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.overlay.backgroundColorHelp }}</p>
        </v-col>
        <v-col cols="3">
            <color-picker v-model="prefs.preferences.overlayBackgroundColor" class="color-picker" :label="zhCN.common.background"></color-picker>
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
