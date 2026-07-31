<!-- Copyright (C) 2022 Intel Corporation -->
<!-- SPDX-License-Identifier: MIT -->

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue';
import { Api } from '@/core/api';
import { signature as preferencesSignature } from '@/core/preferences';
import { signature as loadoutSignature } from '@/core/loadout';
import { type AppInfo } from '@/core/app-info';
import { zhCN } from '@/locales/zh-CN';

interface InfoRow {
  label: string;
  value: string;
}

defineOptions({ name: 'AboutConfigView' });

const appInfo = ref<AppInfo|null>(null);
const errorMessage = ref('');

function boolText(value: boolean): string {
  return value ? zhCN.common.yes : zhCN.common.no;
}

const applicationRows = computed<InfoRow[]>(() => {
  if (appInfo.value === null) {
    return [];
  }
  return [
    { label: zhCN.about.product, value: appInfo.value.productName },
    { label: zhCN.about.productVersion, value: appInfo.value.productVersion },
    { label: zhCN.about.apiVersion, value: appInfo.value.apiVersion },
    { label: zhCN.about.middlewareApiVersion, value: appInfo.value.middlewareApiVersion },
    { label: zhCN.about.preferencesFormat, value: preferencesSignature.version },
    { label: zhCN.about.loadoutFormat, value: loadoutSignature.version },
    { label: zhCN.about.uiDevMode, value: boolText(appInfo.value.devModeEnabled) },
    { label: zhCN.about.chromiumDebugging, value: boolText(appInfo.value.chromiumDebugEnabled) },
    { label: zhCN.about.debugBlocklist, value: boolText(appInfo.value.debugBlocklistEnabled) },
    { label: zhCN.about.logLevel, value: appInfo.value.logLevel },
    { label: zhCN.about.verboseModules, value: appInfo.value.verboseModules },
  ];
});

const buildRows = computed<InfoRow[]>(() => {
  if (appInfo.value === null) {
    return [];
  }
  return [
    { label: zhCN.about.gitHash, value: appInfo.value.buildHash },
    { label: zhCN.about.shortHash, value: appInfo.value.buildHashShort },
    { label: zhCN.about.buildDateTime, value: appInfo.value.buildDateTime },
    { label: zhCN.about.buildConfig, value: appInfo.value.buildConfig },
    { label: zhCN.about.dirtyBuild, value: boolText(appInfo.value.buildDirty) },
  ];
});

const serviceRows = computed<InfoRow[]>(() => {
  if (appInfo.value === null) {
    return [];
  }
  return [
    { label: zhCN.about.serviceBuildId, value: appInfo.value.serviceBuildId },
    { label: zhCN.about.serviceBuildTime, value: appInfo.value.serviceBuildTime },
    { label: zhCN.about.serviceVersion, value: appInfo.value.serviceVersion },
  ];
});

const runtimeRows = computed<InfoRow[]>(() => {
  if (appInfo.value === null) {
    return [];
  }
  return [
    { label: zhCN.about.cefVersion, value: appInfo.value.cefVersion },
    { label: zhCN.about.msvcVersion, value: appInfo.value.msvcVersion },
    { label: zhCN.about.windowsSdk, value: appInfo.value.winSdkVersion },
    { label: zhCN.about.crtVersion, value: appInfo.value.crtVersion },
    { label: zhCN.about.crtRuntime, value: appInfo.value.crtRuntime },
  ];
});

onMounted(async () => {
  try {
    appInfo.value = await Api.getAppInfo();
  }
  catch (e) {
    errorMessage.value = e instanceof Error ? e.message : String(e);
  }
});
</script>

<template>
  <div class="page-wrap">
    <h2 class="mt-5 ml-5 header-top">
      {{ zhCN.about.title }}
    </h2>

    <v-card class="page-card">
      <v-progress-linear v-if="appInfo === null && errorMessage === ''" indeterminate color="primary" class="mt-4"></v-progress-linear>

      <v-alert v-if="errorMessage !== ''" type="error" variant="tonal" class="mt-5">
        {{ errorMessage }}
      </v-alert>

      <template v-if="appInfo !== null">
        <v-card-title class="section-title">{{ zhCN.about.application }}</v-card-title>
        <v-table density="compact" class="info-table">
          <tbody>
            <tr v-for="row in applicationRows" :key="row.label">
              <td class="info-label">{{ row.label }}</td>
              <td class="info-value">{{ row.value }}</td>
            </tr>
          </tbody>
        </v-table>

        <v-card-title class="section-title">{{ zhCN.about.build }}</v-card-title>
        <v-table density="compact" class="info-table">
          <tbody>
            <tr v-for="row in buildRows" :key="row.label">
              <td class="info-label">{{ row.label }}</td>
              <td class="info-value">{{ row.value }}</td>
            </tr>
          </tbody>
        </v-table>

        <v-card-title class="section-title">{{ zhCN.about.service }}</v-card-title>
        <v-table density="compact" class="info-table">
          <tbody>
            <tr v-for="row in serviceRows" :key="row.label">
              <td class="info-label">{{ row.label }}</td>
              <td class="info-value">{{ row.value }}</td>
            </tr>
          </tbody>
        </v-table>

        <v-card-title class="section-title">{{ zhCN.about.runtime }}</v-card-title>
        <v-table density="compact" class="info-table">
          <tbody>
            <tr v-for="row in runtimeRows" :key="row.label">
              <td class="info-label">{{ row.label }}</td>
              <td class="info-value">{{ row.value }}</td>
            </tr>
          </tbody>
        </v-table>
      </template>
    </v-card>
  </div>
</template>

<style scoped>
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
.section-title {
  color: rgba(255, 255, 255, 0.7);
  font-size: 16px;
  padding: 18px 0 6px;
}
.info-table {
  background: transparent;
}
.info-label {
  width: 210px;
  color: rgba(255, 255, 255, 0.7);
  white-space: nowrap;
}
.info-value {
  overflow-wrap: anywhere;
  user-select: text;
}
</style>
