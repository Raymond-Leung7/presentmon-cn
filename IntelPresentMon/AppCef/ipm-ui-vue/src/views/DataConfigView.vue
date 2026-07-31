<!-- Copyright (C) 2022 Intel Corporation -->
<!-- SPDX-License-Identifier: MIT -->

<script setup lang="ts">
import { computed } from 'vue';
import { usePreferencesStore } from '@/stores/preferences';
import { isDevelopment } from '@/core/env-vars';
import DefaultAdapterSelect from '@/components/DefaultAdapterSelect.vue';
import { zhCN } from '@/locales/zh-CN';

const prefs = usePreferencesStore();

const metricPollMessages = computed(() => {
  if (prefs.preferences.metricPollRate % prefs.preferences.overlayDrawRate !== 0) {
    return [zhCN.data.pollingRateRecommendation(prefs.preferences.overlayDrawRate)];
  }
  return [];
});

</script>

<template>
  <div class="page-wrap">
    <h2 class="mt-5 ml-5 header-top">
      {{ zhCN.data.title }}
    </h2>

    <v-card class="page-card">
      <v-row class="mt-5" v-if="isDevelopment()">
        <v-col cols="3">
          {{ zhCN.data.manualEtwFlush }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.manualEtwFlushHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-row>
            <v-col cols="6">
              <v-switch v-model="prefs.preferences.manualEtwFlush" :label="zhCN.common.enable" color="primary"></v-switch>
            </v-col>
          </v-row>
        </v-col>
      </v-row>

      <v-row class="mt-5" v-if="isDevelopment()">
        <v-col cols="3">
          {{ zhCN.data.manualEtwFlushPeriod }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.manualEtwFlushPeriodHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-slider
            v-model="prefs.preferences.etwFlushPeriod"
            :max="1000"
            :min="1"
            :disabled="!prefs.preferences.manualEtwFlush"
            thumb-label="always"
          ></v-slider>
        </v-col>
      </v-row>

      <v-row class="mt-5">
        <v-col cols="3">
          {{ zhCN.data.pollingRate }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.pollingRateHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-slider
            class="metric-poll-rate"
            v-model="prefs.preferences.metricPollRate"
            :max="240"
            :min="1"
            :messages="metricPollMessages"
            thumb-label="always"
            :hide-details="false"
          ></v-slider>
        </v-col>
      </v-row>

      <v-row class="mt-5" v-if="isDevelopment()">
        <v-col cols="3">
          {{ zhCN.data.metricWindowOffset }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.metricWindowOffsetHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-slider
            v-model="prefs.preferences.metricsOffset"
            :max="1500"
            :min="0"
            thumb-label="always"
          ></v-slider>
        </v-col>
      </v-row>

      <v-row class="mt-5">
        <v-col cols="3">
          {{ zhCN.data.telemetryPeriod }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.telemetryPeriodHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-slider
            v-model="prefs.preferences.telemetrySamplingPeriodMs"
            :max="500"
            :min="1"
            thumb-label="always"
          ></v-slider>
        </v-col>
      </v-row>

      <v-row class="mt-8">
        <v-col cols="3">
          {{ zhCN.data.windowSize }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.windowSizeHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-slider
            v-model="prefs.preferences.metricsWindow"
            :max="5000"
            :min="10"
            :step="10"
            thumb-label="always"
          ></v-slider>
        </v-col>
      </v-row>

      <v-row class="mt-8">
        <v-col cols="3">
          {{ zhCN.data.perMetricDevice }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.perMetricDeviceHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <v-row>
            <v-col cols="6">
              <v-switch
                v-model="prefs.preferences.enablePerMetricDeviceSelection"
                :label="zhCN.common.enable"
                color="primary"
              ></v-switch>
            </v-col>
          </v-row>
        </v-col>
      </v-row>

      <v-row class="mt-8">
        <v-col cols="3">
          {{ zhCN.data.defaultAdapter }}
          <p class="text-medium-emphasis text-caption mb-0">
            {{ zhCN.data.defaultAdapterHelp }}
          </p>
        </v-col>
        <v-col cols="9">
          <default-adapter-select></default-adapter-select>
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
  .metric-poll-rate >>> .v-messages__message {
    color: blueviolet;
    padding-left: 10px;
  }
</style>
