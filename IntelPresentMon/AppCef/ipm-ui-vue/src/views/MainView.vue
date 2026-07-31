<script lang="ts" setup>
import { computed, ref, watchEffect } from 'vue'
import { type ListItem } from 'vuetify/lib/composables/list-items.mjs';
import { type Process } from '@/core/process';
import { Action } from '@/core/hotkey';
import { Preset } from '@/core/preferences';
import HotkeyButton from '@/components/HotkeyButton.vue';
import { usePreferencesStore } from '@/stores/preferences';
import { useProcessesStore } from '@/stores/processes';
import { isBlocked } from '@/core/block-list';
import { cancelTopPolling, launchAutotargetting } from '@/core/autotarget';
import { Api } from '@/core/api';
import { zhCN } from '@/locales/zh-CN';

defineOptions({name: 'MainView'})

// === Stores ===
const prefs = usePreferencesStore()
const procs = useProcessesStore()

// === State ===
const loadingProcs = ref(false)

// match autocomplete typed text if substring of window name or process name or pid
function selectFilter(item: Process, query: string) {
    const winText = item.windowName?.toLowerCase();
    if (winText && winText.indexOf(query) > -1) {
        return true;
    }
    return item.name.toLowerCase().indexOf(query) > -1 ||
        item.pid.toString().indexOf(query) > -1;
}
// truncate long process/window names by eliding the middle part
function makeSelectorName(winName: string): string {
    const maxLen = 73;
    const leading = 30;
    const trailing = 40;
    if (winName.length > maxLen) {
     return winName.substr(0, leading) + '...' + winName.substr(-trailing);
    }
    else {
        return winName;
    }
}
// handle click on the capture explore button
async function handleCaptureExplore() {
    await Api.exploreCaptures()
}
// load the process list
async function loadProcesses() {
    loadingProcs.value = true
    await procs.refresh()
    loadingProcs.value = false
    
}

// === Computed ===
const processes = computed(() => {    
    if (prefs.preferences.enableTargetBlocklist) {
        return procs.processes.filter(proc => !isBlocked(proc.name));
    }
    else {
        return procs.processes;
    }
})

// === Watchers ===
// watching selected pid and autotargetting enabled state
watchEffect(async () => {
    const pid = prefs.pid
    if (pid !== null) {
        cancelTopPolling()
        // if the top reported process is not in current list, refresh the list
        if (procs.processes.find(p => p.pid == pid) == null) {
            await procs.refresh()
        }
    }
    else {
        if (prefs.preferences.enableAutotargetting) {
            launchAutotargetting((pid: number) => {
                prefs.pid = pid
            })
        }
        else {
            cancelTopPolling()
        }
    }
})

const processFromItem = (item: ListItem<unknown>) => item.raw as Process;
</script>


<template>
<div class="page-wrap">

    <v-card class="page-card my-5 pt-3">
    <v-row>
        <v-col cols="3">
        {{ zhCN.main.process }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.processHelp }}</p>
        </v-col>
        <v-col cols="9" class="d-flex align-center">
        <v-autocomplete
            :items="processes"
            v-model="prefs.pid"
            item-value="pid"
            :filter="selectFilter"
            :label="zhCN.main.process"
            :loading="loadingProcs"
            @click="loadProcesses"
            append-icon=""
            :disabled="prefs.preferences.enableAutotargetting"
            clearable
        >
            <template v-slot:selection="{ item }">
                <template v-if="processFromItem(item).windowName">
                    {{ makeSelectorName(processFromItem(item).windowName ?? '') }}
                    <span class="pid-node-inline">[{{ processFromItem(item).pid }}]</span>
                </template>            
                <template v-else>
                    <div>
                    {{ processFromItem(item).name }}
                    <span class="pid-node-inline">[{{ processFromItem(item).pid }}]</span>
                    </div>
                </template>
            </template>
            <template v-slot:item="{ item, props }">
                <v-list-item v-if="processFromItem(item).windowName" v-bind="props" :title="makeSelectorName(processFromItem(item).windowName ?? '')">
                    <v-list-item-subtitle>
                        {{ processFromItem(item).name }}
                        <span class="pid-node">[{{ processFromItem(item).pid }}]</span>
                    </v-list-item-subtitle>
                </v-list-item>
                <v-list-item v-else v-bind="props" :title="undefined">
                    <v-list-item-title>
                        {{ makeSelectorName(processFromItem(item).name) }}
                        <span class="pid-node-inline">[{{ processFromItem(item).pid }}]</span>
                    </v-list-item-title>
                </v-list-item>
            </template>
        </v-autocomplete>
        </v-col>
    </v-row> 

    <v-row dense>       
        <v-col cols="3">
        {{ zhCN.main.autoTarget }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.autoTargetHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex align-center">
        <v-switch :label="zhCN.common.enable" v-model="prefs.preferences.enableAutotargetting"></v-switch>
        </v-col>
    </v-row>   
    
    <v-row dense>       
        <v-col cols="3">
        {{ zhCN.main.overlayHotkey }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.overlayHotkeyHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex justify-center align-center">
        <hotkey-button :action="Action.ToggleOverlay"></hotkey-button>
        </v-col>
    </v-row>
    </v-card>
    
    <v-card class="page-card my-5 pt-3">
    <v-row>       
        <v-col cols="3">
        {{ zhCN.main.preset }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.presetHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex justify-center align-center">        
        <v-btn-toggle v-model="prefs.preferences.selectedPreset" :mandatory="prefs.preferences.selectedPreset !== null" variant="outlined" divided>
            <v-btn class="px-5" large>
            {{ zhCN.main.basic }}
            </v-btn>

            <v-btn class="px-5" large>
            {{ zhCN.main.gameExperience }}
            </v-btn>

            <v-btn class="px-5" large>
            {{ zhCN.main.gpuFocus }}
            </v-btn>

            <v-btn class="px-5" large>
            {{ zhCN.main.powerTemperature }}
            </v-btn>

            <v-btn class="px-5" large :value="Preset.Custom">
            {{ zhCN.main.custom }}
            </v-btn>        
        </v-btn-toggle>
        <v-btn
            :to="{name: 'loadout-config'}"
            :disabled="prefs.preferences.selectedPreset !== Preset.Custom"
            color="primary" class="ml-5"
        >
            {{ zhCN.common.edit }}
        </v-btn>
        </v-col>
    </v-row>

    <!-- minimal hotkey component front -->
    <v-row dense>       
        <v-col cols="3">
        {{ zhCN.main.presetCycleHotkey }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.presetCycleHotkeyHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex justify-center align-center">    
        <hotkey-button :action="Action.CyclePreset"></hotkey-button>
        </v-col>  
    </v-row>
    </v-card>
    
    <v-card class="page-card my-5 pt-3">

    <v-row>
        <v-col cols="3">
        {{ zhCN.main.captureDuration }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.captureDurationHelp }}</p>
        </v-col>      
        <v-col cols="2">
            <v-switch v-model="prefs.preferences.enableCaptureDuration" color="primary" :label="zhCN.common.enable" hide-details></v-switch>
        </v-col>
        <v-col cols="3">
        <v-text-field
            :label="zhCN.main.seconds"
            v-model="prefs.preferences.captureDuration"
            :disabled="!prefs.preferences.enableCaptureDuration"
            class="mt-4 ml-8"
            hide-details
            type="number"
            hide-spin-buttons
        ></v-text-field>
        </v-col>
    </v-row>

    <!-- minimal hotkey component front -->
    <v-row dense>       
        <v-col cols="3">
        {{ zhCN.main.captureHotkey }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.captureHotkeyHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex justify-center align-center">
        <hotkey-button :action="Action.ToggleCapture"></hotkey-button>
        </v-col>
    </v-row>

    </v-card>
    
    <v-card class="page-card my-5 pt-3">
    <v-row>       
        <v-col cols="3">
        {{ zhCN.main.captureStorage }}
        <p class="text-medium-emphasis text-caption mb-0">{{ zhCN.main.captureStorageHelp }}</p>
        </v-col>

        <v-col cols="9" class="d-flex justify-center align-center">
        <v-btn 
            large
            color="secondary"
            class="px-6"
            @click="handleCaptureExplore"
        >{{ zhCN.common.openInExplorer }}</v-btn>
        </v-col>
    </v-row>
    </v-card>
    <v-row>
    <v-col cols="12" class="text-right">
        <router-link class="settings-link" :to="{name: 'overlay-config'}">
        {{ zhCN.main.settings }}
        <v-icon large>mdi-cog</v-icon>
        </router-link>
    </v-col>
    </v-row>
</div>
</template>


<style lang="scss" scoped>
.pid-node {
    font-size: 10px;
    color: grey;
    padding-left: 2px;
}
.pid-node-inline {
    font-size: 12px;
    color: grey;
    padding-left: 8px;
}
.page-card {
    margin: 15px 0;
    padding: 0 15px 15px;
}
.page-wrap {
    max-width: 1024px;
}
.hilight-info {
    color: greenyellow;
}
.settings-link {
    font-size: 24px;
    color: #CCC;
    text-decoration: none;
    &:hover {
    color: #FFF
    }
    transition: color .3s;

    i.v-icon.v-icon {
    color: inherit;
    }
}
</style>
