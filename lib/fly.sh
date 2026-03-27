#!/bin/bash
# STAR-XD Fly.io One-Click Deployer
# Usage: bash <(curl -s https://raw.githubusercontent.com/XdKing2/star-xd/main/lib/fly.sh)

set -e

BOLD='\033[1m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════╗"
echo "║        STAR-XD Fly.io Deployer        ║"
echo "║          by XdKing2           ║"
echo "╚═══════════════════════════════════════╝"
echo -e "${NC}"

# Install flyctl if not present
if ! command -v fly &>/dev/null; then
    echo -e "${YELLOW}📦 Installing flyctl...${NC}"
    curl -L https://fly.io/install.sh | sh
    export PATH="$HOME/.fly/bin:$PATH"
fi

# Check login
if ! fly auth whoami &>/dev/null; then
    echo -e "${YELLOW}🔑 Please login to Fly.io:${NC}"
    fly auth login
fi

echo ""
echo -e "${BOLD}📋 Enter your bot details:${NC}"
echo ""

read -p "$(echo -e ${CYAN}App name (e.g. my-star-xd): ${NC})" APP_NAME
APP_NAME=${APP_NAME:-star-xd-bot}

read -p "$(echo -e ${CYAN}Session ID (star~xxxxx): ${NC})" SESSION_ID
if [ -z "$SESSION_ID" ]; then
    echo -e "${RED}❌ Session ID is required!${NC}"
    exit 1
fi

read -p "$(echo -e ${CYAN}Owner WhatsApp number (e.g. 263712345678): ${NC})" OWNER_NUMBER
OWNER_NUMBER=${OWNER_NUMBER:-263710000000}

read -p "$(echo -e ${CYAN}Bot name (default: STAR-XD): ${NC})" BOT_NAME
BOT_NAME=${BOT_NAME:-STAR-XD}

read -p "$(echo -e ${CYAN}MongoDB URL (recommended, press Enter to skip): ${NC})" MONGO_URL

read -p "$(echo -e ${CYAN}Timezone (default: Africa/Harare): ${NC})" TIMEZONE
TIMEZONE=${TIMEZONE:-Africa/Harare}

read -p "$(echo -e ${CYAN}Region (default: iad = US East, options: lhr sin bom): ${NC})" REGION
REGION=${REGION:-iad}

echo ""
echo -e "${YELLOW}🚀 Starting deployment...${NC}"
echo ""

# Clone if not in repo
if [ ! -f "fly.toml" ]; then
    echo -e "${YELLOW}📦 Cloning STAR-XD repo...${NC}"
    git clone https://github.com/XdKing2/star-xd star-xd-deploy
    cd star-xd-deploy
fi

# Update app name and region in fly.toml
sed -i "s/^app = .*/app = \"${APP_NAME}\"/" fly.toml
sed -i "s/^primary_region = .*/primary_region = \"${REGION}\"/" fly.toml

# Launch app (no deploy yet)
echo -e "${YELLOW}📱 Creating Fly.io app: ${APP_NAME}${NC}"
fly launch --no-deploy --name "$APP_NAME" --region "$REGION" --yes 2>/dev/null || true

# Set secrets
echo -e "${YELLOW}⚙️ Setting secrets...${NC}"
fly secrets set \
    SESSION_ID="$SESSION_ID" \
    OWNER_NUMBER="$OWNER_NUMBER" \
    BOT_NAME="$BOT_NAME" \
    TIMEZONE="$TIMEZONE" \
    COMMAND_MODE="public" \
    --app "$APP_NAME"

[ -n "$MONGO_URL" ] && fly secrets set MONGO_URL="$MONGO_URL" --app "$APP_NAME"

# Deploy
echo -e "${YELLOW}📤 Deploying to Fly.io (this may take 3-5 minutes)...${NC}"
fly deploy --app "$APP_NAME" --dockerfile lib/Dockerfile

echo ""
echo -e "${GREEN}╔═══════════════════════════════════════╗${NC}"
echo -e "${GREEN}║        ✅ Deployment Complete!        ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════╝${NC}"
echo ""
echo -e "${BOLD}Logs:${NC}    fly logs -a ${APP_NAME}"
echo -e "${BOLD}Status:${NC}  fly status -a ${APP_NAME}"
echo -e "${BOLD}Stop:${NC}    fly scale count 0 -a ${APP_NAME}"
echo -e "${BOLD}Restart:${NC} fly machine restart -a ${APP_NAME}"
echo ""
echo -e "${CYAN}📱 Scan QR or use Session ID to connect your WhatsApp!${NC}"
